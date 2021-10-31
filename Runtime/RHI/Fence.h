#pragma once
#include "Core/RefPtr.hpp"
#include "GfxDevice/Vulkan/VulkanFence.h"
#include "Types.h"

using namespace GfxDevice::Vulkan;

namespace Sailor::RHI
{
	class Mesh;

	class Fence : virtual public Resource, public IVisitor
	{
	public:
#if defined(VULKAN)
		struct
		{
			TRefPtr<VulkanFence> m_fence;
		} m_vulkan;
#endif

		SAILOR_API void Wait(uint64_t timeout = UINT64_MAX) const;
		SAILOR_API void Reset() const;
		SAILOR_API bool IsFinished() const;
	};
};
