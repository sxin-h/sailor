#include "VulkanBuffer.h"
#include "Core/RefPtr.hpp"
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"
#include "VulkanDeviceMemory.h"

using namespace Sailor;
using namespace Sailor::GfxDevice::Vulkan;

VulkanBuffer::VulkanBuffer(TRefPtr<VulkanDevice> device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode) :
	m_size(size),
	m_usage(usage),
	m_sharingMode(sharingMode),
	m_device(device)
{
}

void VulkanBuffer::Release()
{
	if (m_buffer)
	{
		vkDestroyBuffer(*m_device, m_buffer, nullptr);
	}	
}

void VulkanBuffer::Compile()
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.flags = m_flags;
	bufferInfo.size = m_size;
	bufferInfo.usage = m_usage;
	bufferInfo.sharingMode = m_sharingMode;

	const auto& queues = m_device->GetQueueFamilies();

	uint32_t queueFamily[] = { queues.m_graphicsFamily.value() };
	uint32_t queueFamilies[] = { queues.m_graphicsFamily.value(), queues.m_transferFamily.value() };

	if (m_sharingMode == VK_SHARING_MODE_CONCURRENT)
	{
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queueFamilies;
	}
	else
	{
		bufferInfo.queueFamilyIndexCount = 1;
		bufferInfo.pQueueFamilyIndices = queueFamily;
	}

	VK_CHECK(vkCreateBuffer(*m_device, &bufferInfo, nullptr, &m_buffer));
}

VulkanBuffer::~VulkanBuffer()
{
	Release();
}

VkMemoryRequirements VulkanBuffer::GetMemoryRequirements() const
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(*m_device, m_buffer, &memRequirements);
	return memRequirements;
}

VkResult VulkanBuffer::Bind(TRefPtr<VulkanDeviceMemory> deviceMemory, VkDeviceSize memoryOffset)
{
	VkResult result = vkBindBufferMemory(*m_device, m_buffer, *deviceMemory, memoryOffset);
	if (result == VK_SUCCESS)
	{
		m_deviceMemory = deviceMemory;
		m_memoryOffset = memoryOffset;
	}

	return result;
}
