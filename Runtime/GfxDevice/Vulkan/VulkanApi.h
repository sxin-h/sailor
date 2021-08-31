#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <cassert>
#include <vulkan/vulkan_core.h>
#include "Sailor.h"
#include "Core/RefPtr.hpp"
#include "Core/Singleton.hpp"

class Sailor::Window;

namespace Sailor::GfxDevice::Vulkan
{
	class VulkanCommandBuffer;
	class VulkanCommandPool;
	class VulkanQueue;
	class VulkanDevice;
	class VulkanFence;
	class VulkanSemaphore;
	class VulkanRenderPass;
	class VulkanSwapchain;
	class VulkanSurface;

#define VK_CHECK(call) \
	do { \
		VkResult result_ = call; \
		assert(result_ == VK_SUCCESS); \
	} while (0)

#define NUM_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> m_graphicsFamily;
		std::optional<uint32_t> m_presentFamily;

		SAILOR_API bool IsComplete() const { return m_graphicsFamily.has_value() && m_presentFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR m_capabilities;
		std::vector<VkSurfaceFormatKHR> m_formats;
		std::vector<VkPresentModeKHR> m_presentModes;
	};

	class VulkanApi : public TSingleton<VulkanApi>
	{
	public:
		const int MAX_FRAMES_IN_FLIGHT = 2;

		static SAILOR_API void Initialize(const Window* pViewport, bool bIsDebug);

		static SAILOR_API uint32_t GetNumSupportedExtensions();
		static SAILOR_API void PrintSupportedExtensions();

		static SAILOR_API bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		static SAILOR_API bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);

		static SAILOR_API bool IsDeviceSuitable(VkPhysicalDevice device);
		static SAILOR_API int32_t GetDeviceScore(VkPhysicalDevice device);

		static SAILOR_API void GetRequiredDeviceExtensions(std::vector<const char*>& requiredDeviceExtensions) { requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; }

		static SAILOR_API VkShaderModule CreateShaderModule(const std::vector<uint32_t>& inCode);

		virtual SAILOR_API ~VulkanApi() override;

		void SAILOR_API DrawFrame(Window* pViewport);

		static SAILOR_API void WaitIdle();

		static SAILOR_API QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		static SAILOR_API SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		static SAILOR_API VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static SAILOR_API VkPresentModeKHR —hooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool bVSync);
		static SAILOR_API VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

		static SAILOR_API VkAttachmentDescription GetDefaultColorAttachment(VkFormat imageFormat);
		static SAILOR_API VkAttachmentDescription GetDefaultDepthAttachment(VkFormat depthFormat);

		static SAILOR_API TRefPtr<VulkanRenderPass> CreateRenderPass(VkDevice device, VkFormat imageFormat, VkFormat depthFormat);
		static SAILOR_API TRefPtr<VulkanRenderPass> CreateMSSRenderPass(VkDevice device, VkFormat imageFormat, VkFormat depthFormat, VkSampleCountFlagBits samples);

	private:

		static SAILOR_API bool SetupDebugCallback();
		static SAILOR_API VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const	VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static SAILOR_API void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const	VkAllocationCallbacks* pAllocator);

		static SAILOR_API VkPhysicalDevice PickPhysicalDevice();

		SAILOR_API void CreateLogicalDevice(VkPhysicalDevice physicalDevice);
		SAILOR_API void CreateWin32Surface(const Window* pViewport);
		SAILOR_API void CreateSwapchain(const Window* pViewport);
		SAILOR_API bool RecreateSwapchain(Window* pViewport);
		SAILOR_API void CreateGraphicsPipeline();
		SAILOR_API void CreateRenderPass();
		SAILOR_API void CreateFramebuffers();
		SAILOR_API void CreateCommandPool();
		SAILOR_API void CreateCommandBuffers();
		SAILOR_API void CreateFrameSyncSemaphores();

		SAILOR_API void CleanupSwapChain();

		__forceinline static SAILOR_API VkInstance& GetVkInstance() { return m_pInstance->m_vkInstance; }

		bool bIsEnabledValidationLayers = false;
		VkInstance m_vkInstance = 0;

		// Command pool
		TRefPtr<VulkanCommandPool> m_commandPool;
		std::vector<TRefPtr<VulkanCommandBuffer>> m_commandBuffers;

		// Render Pass
		TRefPtr<VulkanRenderPass> m_renderPass;
		
		// Pipeline
		VkPipelineLayout m_pipelineLayout = 0;
		VkPipeline m_graphicsPipeline = 0;

		// Queuees
		TRefPtr<VulkanQueue> m_graphicsQueue;
		TRefPtr<VulkanQueue> m_presentQueue;

		TRefPtr<VulkanSurface> m_surface;

		VkDebugUtilsMessengerEXT m_debugMessenger = 0;

		VkPhysicalDevice m_mainPhysicalDevice = 0;
		VkDevice m_device = 0;
		QueueFamilyIndices m_queueFamilies;

		// Swapchain
		TRefPtr<VulkanSwapchain> m_swapchain;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;

		// Frame sync
		std::vector<TRefPtr<VulkanSemaphore>> m_imageAvailableSemaphores;
		std::vector<TRefPtr<VulkanSemaphore>> m_renderFinishedSemaphores;
		std::vector<TRefPtr<VulkanFence>> m_syncFences;
		std::vector<TRefPtr<VulkanFence>> m_syncImages;
		size_t m_currentFrame = 0;

		std::atomic<bool> bIsFramebufferResizedThisFrame = false;
	};
}
