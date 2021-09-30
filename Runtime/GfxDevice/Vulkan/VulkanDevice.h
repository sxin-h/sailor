#pragma once
#include "VulkanApi.h"
#include "VulkanDescriptors.h"
#include "VulkanSamplers.h"
#include "Core/RefPtr.hpp"
#include "RHI/RHIResource.h"

class Sailor::Win32::Window;

namespace Sailor::GfxDevice::Vulkan
{
	class VulkanDescriptorPool;
	class VulkanDescriptorSetLayout;
	class VulkanDescriptorSet;
	class VulkanCommandBuffer;
	class VulkanCommandPool;
	class VulkanQueue;
	class VulkanDevice;
	class VulkanFence;
	class VulkanSemaphore;
	class VulkanRenderPass;
	class VulkanSwapchain;
	class VulkanSurface;
	class VulkanFramebuffer;
	class VulkanDeviceMemory;
	class VulkanBuffer;
	class VulkanPipeline;
	class VulkanPipelineLayout;
	class VulkanSampler;

	class VulkanDevice final : public RHI::RHIResource
	{

	public:

		operator VkDevice() const { return m_device; }

		VulkanDevice(const Win32::Window* pViewport, RHI::EMSAASamples requestMsaa);
		virtual ~VulkanDevice();
		void Shutdown();

		SAILOR_API VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }

		void SAILOR_API WaitIdle();
		void SAILOR_API WaitIdlePresentQueue();
		bool SAILOR_API PresentFrame(const std::vector<TRefPtr<VulkanCommandBuffer>>* primaryCommandBuffers = nullptr,
			const std::vector<TRefPtr<VulkanCommandBuffer>>* secondaryCommandBuffers = nullptr,
			const std::vector<TRefPtr<VulkanSemaphore>>* waitSemaphores = nullptr);

		bool SAILOR_API IsSwapChainOutdated() const { return m_bIsSwapChainOutdated; }

		SAILOR_API TRefPtr<VulkanSurface> GetSurface() const;
		SAILOR_API TRefPtr<VulkanCommandBuffer> CreateCommandBuffer(bool bOnlyTransferQueue = false);

		SAILOR_API void SubmitCommandBuffer(TRefPtr<VulkanCommandBuffer> commandBuffer,
			TRefPtr<VulkanFence> fence = nullptr,
			std::vector<TRefPtr<VulkanSemaphore>> signalSemaphores = {},
			std::vector<TRefPtr<VulkanSemaphore>> waitSemaphores = {}) const;

		SAILOR_API const VulkanQueueFamilyIndices& GetQueueFamilies() const { return m_queueFamilies; }

		SAILOR_API bool ShouldFixLostDevice(const Win32::Window* pViewport);
		SAILOR_API void FixLostDevice(const Win32::Window* pViewport);

		SAILOR_API float GetMaxAllowedAnisotropy() const { return m_maxAllowedAnisotropy; };
		SAILOR_API VkSampleCountFlagBits GetMaxAllowedMSAASamples() const { return m_maxAllowedMSAASamples; };
		SAILOR_API VkSampleCountFlagBits GetCurrentMSAASamples() const { return m_currentMSAASamples; };

		SAILOR_API VkFormat GetDepthFormat() const;
		SAILOR_API bool IsMipsSupported(VkFormat format) const;

	protected:

		SAILOR_API void CreateLogicalDevice(VkPhysicalDevice physicalDevice);
		SAILOR_API void CreateWin32Surface(const Win32::Window* pViewport);
		SAILOR_API void CreateSwapchain(const Win32::Window* pViewport);
		SAILOR_API bool RecreateSwapchain(const Win32::Window* pViewport);
		SAILOR_API void CreateGraphicsPipeline();
		SAILOR_API void CreateRenderPass();
		SAILOR_API void CreateFramebuffers();
		SAILOR_API void CreateCommandPool();
		SAILOR_API void CreateCommandBuffers();
		SAILOR_API void CreateFrameSyncSemaphores();
		SAILOR_API void CleanupSwapChain();
		SAILOR_API void CreateVertexBuffer();

		float m_maxAllowedAnisotropy = 0;
		VkSampleCountFlagBits m_maxAllowedMSAASamples = VK_SAMPLE_COUNT_1_BIT;
		VkSampleCountFlagBits m_currentMSAASamples = VK_SAMPLE_COUNT_1_BIT;

		// Command pool
		TRefPtr<VulkanCommandPool> m_commandPool;
		TRefPtr<VulkanCommandPool> m_transferCommandPool;

		std::vector<TRefPtr<VulkanCommandBuffer>> m_commandBuffers;

		TRefPtr<VulkanDescriptorPool> m_descriptorPool;

		// Render Pass
		TRefPtr<VulkanRenderPass> m_renderPass;

		// Queuees
		TRefPtr<VulkanQueue> m_graphicsQueue;
		TRefPtr<VulkanQueue> m_presentQueue;
		TRefPtr<VulkanQueue> m_transferQueue;

		TRefPtr<VulkanSurface> m_surface;

		VkPhysicalDevice m_physicalDevice = 0;
		VkDevice m_device = 0;
		VulkanQueueFamilyIndices m_queueFamilies;

		// Swapchain
		TRefPtr<VulkanSwapchain> m_swapchain;
		std::vector<TRefPtr<VulkanFramebuffer>> m_swapChainFramebuffers;

		// Frame sync
		std::vector<TRefPtr<VulkanSemaphore>> m_imageAvailableSemaphores;
		std::vector<TRefPtr<VulkanSemaphore>> m_renderFinishedSemaphores;
		std::vector<TRefPtr<VulkanFence>> m_syncFences;
		std::vector<TRefPtr<VulkanFence>> m_syncImages;
		size_t m_currentFrame = 0;

		std::atomic<bool> m_bIsSwapChainOutdated = true;

		VulkanSamplers m_samplers;

		// Custom testing code
		TRefPtr<VulkanPipelineLayout> m_pipelineLayout = nullptr;
		TRefPtr<VulkanPipeline> m_graphicsPipeline = nullptr;
		TRefPtr<VulkanDescriptorSetLayout> m_descriptorSetLayout = nullptr;

		TRefPtr<VulkanBuffer> m_vertexBuffer;
		TRefPtr<VulkanBuffer> m_indexBuffer;
		TRefPtr<VulkanBuffer> m_uniformBuffer;
		TRefPtr<VulkanDescriptorSet> m_descriptorSet;

		TRefPtr<VulkanImage> m_image;
		TRefPtr<VulkanImageView> m_imageView;
	};
}