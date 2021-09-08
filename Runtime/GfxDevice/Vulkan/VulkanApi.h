#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Sailor.h"
#include "Core/RefPtr.hpp"
#include "Core/Singleton.hpp"

class Sailor::Win32::Window;
using namespace Sailor::Win32;

namespace Sailor::GfxDevice::Vulkan
{
	class VulkanRenderPass;
	class VulkanSurface;
	class VulkanDevice;
	class VulkanImageView;
	class VulkanImage;

#define VK_CHECK(call) \
	do { \
		VkResult result_ = call; \
		assert(result_ == VK_SUCCESS); \
	} while (0)

#define NUM_ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

	struct VulkanQueueFamilyIndices
	{
		std::optional<uint32_t> m_graphicsFamily;
		std::optional<uint32_t> m_presentFamily;
		std::optional<uint32_t> m_transferFamily;

		SAILOR_API bool IsComplete() const { return m_graphicsFamily.has_value() && m_presentFamily.has_value() && m_transferFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR m_capabilities;
		std::vector<VkSurfaceFormatKHR> m_formats;
		std::vector<VkPresentModeKHR> m_presentModes;
	};

	class RHIVertexFactoryPositionColor
	{
	public:

		static SAILOR_API VkVertexInputBindingDescription GetBindingDescription();
		static SAILOR_API std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
	};

	class VulkanApi : public TSingleton<VulkanApi>
	{
	public:
		static constexpr int MaxFramesInFlight = 2;

		static SAILOR_API void Initialize(const Window* pViewport, bool bIsDebug);
		virtual SAILOR_API ~VulkanApi() override;

		static void SAILOR_API DrawFrame(Window* pViewport);
		static void SAILOR_API WaitIdle();
		SAILOR_API TRefPtr<VulkanDevice> GetMainDevice() const;

		bool SAILOR_API IsEnabledValidationLayers() const { return bIsEnabledValidationLayers; }
		__forceinline static SAILOR_API VkInstance& GetVkInstance() { return m_pInstance->m_vkInstance; }

		static SAILOR_API VulkanQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, TRefPtr<VulkanSurface> surface);
		static SAILOR_API SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, TRefPtr<VulkanSurface> surface);

		static SAILOR_API VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static SAILOR_API VkPresentModeKHR —hooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool bVSync);
		static SAILOR_API VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

		static SAILOR_API VkAttachmentDescription GetDefaultColorAttachment(VkFormat imageFormat);
		static SAILOR_API VkAttachmentDescription GetDefaultDepthAttachment(VkFormat depthFormat);

		static SAILOR_API TRefPtr<VulkanRenderPass> CreateRenderPass(VkDevice device, VkFormat imageFormat, VkFormat depthFormat);
		static SAILOR_API TRefPtr<VulkanRenderPass> CreateMSSRenderPass(VkDevice device, VkFormat imageFormat, VkFormat depthFormat, VkSampleCountFlagBits samples);

		static SAILOR_API VkPhysicalDevice PickPhysicalDevice(TRefPtr<VulkanSurface> surface);

		static SAILOR_API void GetRequiredDeviceExtensions(std::vector<const char*>& requiredDeviceExtensions) { requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; }

		//static SAILOR_API VkShaderModule CreateShaderModule(const std::vector<uint32_t>& inCode);

		static SAILOR_API VkImageAspectFlags ComputeAspectFlagsForFormat(VkFormat format);
		static SAILOR_API TRefPtr<VulkanImageView> CreateImageView(VkDevice device, TRefPtr<VulkanImage> image, VkImageAspectFlags aspectFlags);

		static SAILOR_API uint32_t FindMemoryByType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	private:

		static SAILOR_API uint32_t GetNumSupportedExtensions();
		static SAILOR_API void PrintSupportedExtensions();

		static SAILOR_API bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		static SAILOR_API bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);

		static SAILOR_API bool IsDeviceSuitable(VkPhysicalDevice device, TRefPtr<VulkanSurface> surface);
		static SAILOR_API int32_t GetDeviceScore(VkPhysicalDevice device);

		static SAILOR_API bool SetupDebugCallback();
		static SAILOR_API VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const	VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static SAILOR_API void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const	VkAllocationCallbacks* pAllocator);

		VkDebugUtilsMessengerEXT m_debugMessenger = 0;
		bool bIsEnabledValidationLayers = false;

		VkInstance m_vkInstance = 0;
		TRefPtr<VulkanDevice> m_device;
	};
}
