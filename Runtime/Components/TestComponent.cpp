#include "AssetRegistry/FrameGraph/FrameGraphImporter.h"
#include "AssetRegistry/Material/MaterialImporter.h"
#include "AssetRegistry/Texture/TextureImporter.h"
#include "Components/TestComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/CameraComponent.h"
#include "Engine/GameObject.h"
#include "Engine/EngineLoop.h"
#include "ECS/TransformECS.h"

using namespace Sailor;
using namespace Sailor::Tasks;

void TestComponent::BeginPlay()
{
	GetWorld()->GetDebugContext()->DrawOrigin(glm::vec4(0, 2, 0, 0), 20.0f, 1000.0f);

	for (int32_t i = -1000; i < 1000; i += 32)
	{
		for (int32_t j = -1000; j < 1000; j += 32)
		{
			int k = 10;
			//for (int32_t k = -1000; k < 1000; k += 32) 
			{
				m_boxes.Add(Math::AABB(glm::vec3(i, k, j), glm::vec3(1.0f, 1.0f, 1.0f)));
				const auto& aabb = m_boxes[m_boxes.Num() - 1];

				GetWorld()->GetDebugContext()->DrawAABB(aabb, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f), 5.0f);

				m_octree.Insert(aabb.GetCenter(), aabb.GetExtents(), aabb);
			}
		}
	}

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			auto gameObject2 = GetWorld()->Instantiate();
			gameObject2->GetTransformComponent().SetPosition(vec3(j * 3000, 0, i * 2500));
			gameObject2->AddComponent<MeshRendererComponent>();
		}

	auto gameObject3 = GetWorld()->Instantiate();
	gameObject3->GetTransformComponent().SetPosition(vec3(0, 0, 0));
	gameObject3->AddComponent<MeshRendererComponent>();
	//
	//m_octree.DrawOctree(*GetWorld()->GetDebugContext(), 10);
}

void TestComponent::EndPlay()
{
}

void TestComponent::Tick(float deltaTime)
{
	auto& transform = GetOwner()->GetTransformComponent();
	const vec3 cameraViewDirection = transform.GetRotation() * Math::vec4_Forward;

	const float sensitivity = 500;

	glm::vec3 delta = glm::vec3(0.0f, 0.0f, 0.0f);
	if (GetWorld()->GetInput().IsKeyDown('A'))
		delta += -cross(cameraViewDirection, Math::vec3_Up);

	if (GetWorld()->GetInput().IsKeyDown('D'))
		delta += cross(cameraViewDirection, Math::vec3_Up);

	if (GetWorld()->GetInput().IsKeyDown('W'))
		delta += cameraViewDirection;

	if (GetWorld()->GetInput().IsKeyDown('S'))
		delta += -cameraViewDirection;

	if (GetWorld()->GetInput().IsKeyDown('X'))
		delta += vec3(1, 0, 0);

	if (GetWorld()->GetInput().IsKeyDown('Y'))
		delta += vec3(0, 1, 0);

	if (GetWorld()->GetInput().IsKeyDown('Z'))
		delta += vec3(0, 0, 1);

	if (glm::length(delta) > 0)
	{
		const vec4 newPosition = transform.GetPosition() + vec4(glm::normalize(delta) * sensitivity * deltaTime, 1.0f);
		transform.SetPosition(newPosition);
	}

	if (GetWorld()->GetInput().IsKeyDown(VK_LBUTTON))
	{
		const float speed = 1.0f;
		const vec2 shift = vec2(GetWorld()->GetInput().GetCursorPos() - m_lastCursorPos) * speed;

		m_yaw += -shift.x;
		m_pitch = glm::clamp(m_pitch - shift.y, -85.0f, 85.0f);

		glm::quat hRotation = angleAxis(glm::radians(m_yaw), Math::vec3_Up);
		glm::quat vRotation = angleAxis(glm::radians(m_pitch), hRotation * Math::vec3_Right);

		transform.SetRotation(vRotation * hRotation);
	}

	if (GetWorld()->GetInput().IsKeyPressed('F'))
	{
		if (auto camera = GetOwner()->GetComponent<CameraComponent>())
		{
			m_cachedFrustum = transform.GetTransform().Matrix();

			Math::Frustum frustum;
			frustum.ExtractFrustumPlanes(transform.GetTransform(), camera->GetAspect(), camera->GetFov(), camera->GetZNear(), camera->GetZFar());
			m_octree.Trace(frustum, m_culledBoxes);
		}
	}

	for (const auto& aabb : m_culledBoxes)
	{
		GetWorld()->GetDebugContext()->DrawAABB(aabb, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));
	}

	if (auto camera = GetOwner()->GetComponent<CameraComponent>())
	{
		GetWorld()->GetDebugContext()->DrawFrustum(m_cachedFrustum, camera->GetFov(), 500.0f, camera->GetZNear(), camera->GetAspect(), glm::vec4(1.0, 1.0, 0.0, 1.0f));
	}

	m_lastCursorPos = GetWorld()->GetInput().GetCursorPos();

	static bool bTrigger = true;
	if (bTrigger && GetWorld()->GetInput().IsKeyPressed('R'))
	{
		bTrigger = false;

		glm::vec4 colors[] = { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) , glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) };

		for (auto& go : GetWorld()->GetGameObjects())
		{
			if (auto mr = go->GetComponent<MeshRendererComponent>())
			{
				for (auto& mat : mr->GetMaterials())
				{
					if (mat && mat->IsReady() && mat->GetShaderBindings()->HasParameter("material.color"))
					{
						mat = Material::CreateInstance(GetWorld(), mat);

						App::GetSubmodule<Sailor::RHI::Renderer>()->GetDriverCommands()->SetMaterialParameter(GetWorld()->GetCommandList(),
							mat->GetShaderBindings(), "material.color", colors[rand() % 3]);
					}

				}

			}
		}

	}
}


