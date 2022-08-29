#include "RHIRenderSceneNode.h"
#include "RHI/SceneView.h"
#include "RHI/Renderer.h"
#include "RHI/Shader.h"
#include "RHI/Texture.h"

using namespace Sailor;
using namespace Sailor::RHI;

#ifndef _SAILOR_IMPORT_
const char* RHIRenderSceneNode::m_name = "RenderScene";
#endif

RHI::ESortingOrder RHIRenderSceneNode::GetSortingOrder() const
{
	const std::string& sortOrder = GetStringParam("Sorting");

	if (!sortOrder.empty())
	{
		return magic_enum::enum_cast<RHI::ESortingOrder>(sortOrder).value_or(RHI::ESortingOrder::FrontToBack);
	}

	return RHI::ESortingOrder::FrontToBack;
}

struct PerInstanceData
{
	alignas(16) glm::mat4 model;
	alignas(16) uint32_t materialInstance = 0;

	bool operator==(const PerInstanceData& rhs) const { return this->materialInstance == rhs.materialInstance && this->model == rhs.model; }

	size_t GetHash() const
	{
		hash<glm::mat4> p;
		return p(model);
	}
};

class Batch
{
public:

	RHIMaterialPtr m_material;

	Batch() = default;
	Batch(const RHIMaterialPtr& material) : m_material(material) {}

	bool operator==(const Batch& rhs) const
	{
		return
			m_material->GetBindings()->GetCompatibilityHashCode() == rhs.m_material->GetBindings()->GetCompatibilityHashCode() &&
			m_material->GetVertexShader() == rhs.m_material->GetVertexShader() &&
			m_material->GetFragmentShader() == rhs.m_material->GetFragmentShader() &&
			m_material->GetRenderState() == rhs.m_material->GetRenderState();
	}

	size_t GetHash() const
	{
		return m_material->GetBindings()->GetCompatibilityHashCode();
	}
};

/*
https://developer.nvidia.com/vulkan-shader-resource-binding
*/
void RHIRenderSceneNode::Process(RHIFrameGraph* frameGraph, RHI::RHICommandListPtr transferCommandList, RHI::RHICommandListPtr commandList, const RHI::RHISceneViewSnapshot& sceneView)
{
	SAILOR_PROFILE_FUNCTION();

	auto scheduler = App::GetSubmodule<Tasks::Scheduler>();
	auto& driver = App::GetSubmodule<RHI::Renderer>()->GetDriver();
	auto commands = App::GetSubmodule<RHI::Renderer>()->GetDriverCommands();

	TMap<Batch, TMap<RHI::RHIMeshPtr, TVector<PerInstanceData>>> drawCalls;
	TSet<Batch> batches;

	uint32_t numMeshes = 0;

	SAILOR_PROFILE_BLOCK("Filter sceneView by tag");
	for (auto& proxy : sceneView.m_proxies)
	{
		for (size_t i = 0; i < proxy.m_meshes.Num(); i++)
		{
			const bool bHasMaterial = proxy.GetMaterials().Num() > i;
			if (!bHasMaterial)
			{
				break;
			}

			const auto& mesh = proxy.m_meshes[i];
			const auto& material = proxy.GetMaterials()[i];

			const bool bIsMaterialReady = material &&
				material->GetVertexShader() &&
				material->GetFragmentShader() &&
				material->GetBindings() &&
				material->GetBindings()->GetShaderBindings().Num() > 0;

			if (!bIsMaterialReady)
			{
				continue;
			}

			if (material->GetRenderState().GetTag() == GetHash(GetStringParam("Tag")))
			{
				RHIShaderBindingPtr shaderBinding;
				if (material->GetBindings()->GetShaderBindings().ContainsKey("material"))
				{
					shaderBinding = material->GetBindings()->GetShaderBindings()["material"];
				}

				PerInstanceData data;
				data.model = proxy.m_worldMatrix;
				data.materialInstance = shaderBinding.IsValid() ? shaderBinding->GetStorageInstanceIndex() : 0;

				Batch batch(material);

				drawCalls[batch][mesh].Add(data);
				batches.Insert(batch);

				numMeshes++;
			}
		}
	}
	SAILOR_PROFILE_END_BLOCK();

	SAILOR_PROFILE_BLOCK("Create storage for matrices");
	RHI::RHIShaderBindingSetPtr perInstanceData = Sailor::RHI::Renderer::GetDriver()->CreateShaderBindings();
	RHI::RHIShaderBindingPtr storageBinding = Sailor::RHI::Renderer::GetDriver()->AddSsboToShaderBindings(perInstanceData, "data", sizeof(PerInstanceData), numMeshes, 0);
	SAILOR_PROFILE_END_BLOCK();

	SAILOR_PROFILE_BLOCK("Prepare command list");
	TVector<PerInstanceData> gpuMatricesData;
	gpuMatricesData.AddDefault(numMeshes);

	auto colorAttachment = GetResourceParam("color").StaticCast<RHI::RHITexture>();
	if (!colorAttachment)
	{
		colorAttachment = frameGraph->GetRenderTarget("BackBuffer");
	}

	auto depthAttachment = GetResourceParam("depthStencil").StaticCast<RHI::RHITexture>();
	if (!depthAttachment)
	{
		depthAttachment = frameGraph->GetRenderTarget("DepthBuffer");
	}

	const size_t numThreads = scheduler->GetNumRHIThreads() + 1;
	const size_t materialsPerThread = (batches.Num()) / numThreads;

	TVector<RHICommandListPtr> secondaryCommandLists(batches.Num() > numThreads ? (numThreads - 1) : 0);
	TVector<Tasks::ITaskPtr> tasks;

	auto vecBatches = batches.ToVector();

	SAILOR_PROFILE_BLOCK("Calculate SSBO offsets");
	size_t ssboIndex = 0;
	TVector<uint32_t> storageIndex(vecBatches.Num());
	for (uint32_t j = 0; j < vecBatches.Num(); j++)
	{
		bool bIsInited = false;
		for (auto& instancedDrawCall : drawCalls[vecBatches[j]])
		{
			auto& mesh = instancedDrawCall.First();
			auto& matrices = instancedDrawCall.Second();

			memcpy(&gpuMatricesData[ssboIndex], matrices.GetData(), sizeof(PerInstanceData) * matrices.Num());

			if (!bIsInited)
			{
				storageIndex[j] = storageBinding->GetStorageInstanceIndex() + (uint32_t)ssboIndex;
				bIsInited = true;
			}
			ssboIndex += matrices.Num();
		}
	}
	SAILOR_PROFILE_END_BLOCK();

	SAILOR_PROFILE_BLOCK("Create secondary command lists");
	for (uint32_t i = 0; i < secondaryCommandLists.Num(); i++)
	{
		const uint32_t start = (uint32_t)materialsPerThread * i;
		const uint32_t end = (uint32_t)materialsPerThread * (i + 1);

		auto task = Tasks::Scheduler::CreateTask("Record draw calls in secondary command list",
			[&, i = i, start = start, end = end]()
		{
			RHICommandListPtr cmdList = driver->CreateCommandList(true, false);
			commands->BeginSecondaryCommandList(cmdList, true, true);
			commands->SetDefaultViewport(cmdList);

			bool bAreBuffersBind = false;

			for (uint32_t j = start; j < end; j++)
			{
				auto& material = vecBatches[j].m_material;

				TVector<RHIShaderBindingSetPtr> sets({ sceneView.m_frameBindings, perInstanceData, material->GetBindings() });

				commands->BindMaterial(cmdList, material);
				commands->BindShaderBindings(cmdList, material, sets);

				uint32_t ssboOffset = 0;
				for (auto& instancedDrawCall : drawCalls[material])
				{
					auto& mesh = instancedDrawCall.First();
					auto& matrices = instancedDrawCall.Second();

					if (!bAreBuffersBind)
					{
						commands->BindVertexBufferEx(cmdList, mesh->m_vertexBuffer, 0);
						commands->BindIndexBufferEx(cmdList, mesh->m_indexBuffer, 0);

						bAreBuffersBind = true;
					}

					// Draw Batch
					commands->DrawIndexed(cmdList, (uint32_t)mesh->m_indexBuffer->GetSize() / sizeof(uint32_t),
						(uint32_t)matrices.Num(),
						(uint32_t)mesh->m_indexBuffer->m_vulkan.m_buffer.m_offset / sizeof(uint32_t),
						(uint32_t)mesh->m_vertexBuffer->m_vulkan.m_buffer.m_offset,
						storageIndex[j] + ssboOffset);

					ssboOffset += (uint32_t)matrices.Num();
				}
			}

			commands->EndCommandList(cmdList);
			secondaryCommandLists[i] = std::move(cmdList);
		}, Tasks::EThreadType::RHI);

		task->Run();
		tasks.Add(task);
	}
	SAILOR_PROFILE_END_BLOCK();

	SAILOR_PROFILE_BLOCK("Fill transfer command list with matrices data");
	if (gpuMatricesData.Num() > 0)
	{
		commands->UpdateShaderBinding(transferCommandList, storageBinding,
			gpuMatricesData.GetData(),
			sizeof(PerInstanceData) * gpuMatricesData.Num(),
			sizeof(PerInstanceData) * storageBinding->GetStorageInstanceIndex());
	}
	SAILOR_PROFILE_END_BLOCK();

	commands->ImageMemoryBarrier(commandList, colorAttachment, colorAttachment->GetFormat(), colorAttachment->GetDefaultLayout(), EImageLayout::ColorAttachmentOptimal);

	SAILOR_PROFILE_BLOCK("Record draw calls in primary command list");
	commands->BeginRenderPass(commandList,
		TVector<RHI::RHITexturePtr>{ colorAttachment },
		depthAttachment,
		glm::vec4(0, 0, colorAttachment->GetExtent().x, colorAttachment->GetExtent().y),
		glm::ivec2(0, 0),
		true,
		glm::vec4(0.0f),
		true);

	for (size_t i = secondaryCommandLists.Num() * materialsPerThread; i < vecBatches.Num(); i++)
	{
		auto& material = vecBatches[i].m_material;

		TVector<RHIShaderBindingSetPtr> sets({ sceneView.m_frameBindings, perInstanceData, material->GetBindings() });

		commands->BindMaterial(commandList, material);
		commands->BindShaderBindings(commandList, material, sets);

		bool bAreBuffersBind = false;
		uint32_t ssboOffset = 0;
		for (auto& instancedDrawCall : drawCalls[material])
		{
			auto& mesh = instancedDrawCall.First();
			auto& matrices = instancedDrawCall.Second();

			if (!bAreBuffersBind)
			{
				commands->BindVertexBufferEx(commandList, mesh->m_vertexBuffer, 0);
				commands->BindIndexBufferEx(commandList, mesh->m_indexBuffer, 0);

				bAreBuffersBind = true;
			}

			// Draw Batch
			commands->DrawIndexed(commandList, (uint32_t)mesh->m_indexBuffer->GetSize() / sizeof(uint32_t),
				(uint32_t)matrices.Num(),
				(uint32_t)mesh->m_indexBuffer->m_vulkan.m_buffer.m_offset / sizeof(uint32_t),
				(uint32_t)mesh->m_vertexBuffer->m_vulkan.m_buffer.m_offset,
				storageIndex[i] + ssboOffset);

			ssboOffset += (uint32_t)matrices.Num();
		}
	}
	commands->EndRenderPass(commandList);
	SAILOR_PROFILE_END_BLOCK();

	SAILOR_PROFILE_BLOCK("Wait for secondary command lists");
	for (auto& task : tasks)
	{
		task->Wait();
	}
	SAILOR_PROFILE_END_BLOCK();

	commands->RenderSecondaryCommandBuffers(commandList,
		secondaryCommandLists,
		TVector<RHI::RHITexturePtr>{ colorAttachment },
		depthAttachment,
		glm::vec4(0, 0, colorAttachment->GetExtent().x, colorAttachment->GetExtent().y),
		glm::ivec2(0, 0),
		false,
		glm::vec4(0.0f),
		true);

	commands->ImageMemoryBarrier(commandList, colorAttachment, colorAttachment->GetFormat(), EImageLayout::ColorAttachmentOptimal, colorAttachment->GetDefaultLayout());

	SAILOR_PROFILE_END_BLOCK();
}

void RHIRenderSceneNode::Clear()
{
}