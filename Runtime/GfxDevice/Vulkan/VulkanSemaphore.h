#pragma once
#include "RHI/RHIResource.h"
#include "VulkanApi.h"

using namespace Sailor::RHI;

namespace Sailor::GfxDevice::Vulkan
{
	class VulkanSemaphore : public Sailor::RHI::RHIResource
	{

	public:

		VulkanSemaphore(VkDevice device, VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, void* pNextCreateInfo = nullptr);

		operator VkSemaphore() const { return m_semaphore; }

		VkPipelineStageFlags& pipelineStageFlags() { return m_pipelineStageFlags; }
		const VkPipelineStageFlags& pipelineStageFlags() const { return m_pipelineStageFlags; }
		const VkSemaphore* GetHandle() const { return &m_semaphore; }

	protected:
		virtual ~VulkanSemaphore();

		VkSemaphore m_semaphore;
		VkPipelineStageFlags m_pipelineStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		VkDevice m_device;
	};
}