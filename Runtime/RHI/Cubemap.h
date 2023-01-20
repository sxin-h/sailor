#pragma once
#include "Memory/RefPtr.hpp"

#if defined(SAILOR_BUILD_WITH_VULKAN)
#include "GraphicsDriver/Vulkan/VulkanGraphicsDriver.h"
#endif

#include "Texture.h"
#include "Types.h"

using namespace GraphicsDriver::Vulkan;

namespace Sailor::RHI
{
	class RHICubemap: public RHITexture
	{
	public:

		SAILOR_API RHICubemap(ETextureFiltration filtration, ETextureClamping clamping, bool bShouldGenerateMips, EImageLayout defaultLayout = EImageLayout::ShaderReadOnlyOptimal) :
			RHITexture(filtration, clamping, bShouldGenerateMips, defaultLayout)
		{}

		RHIRenderTargetPtr GetFace(uint32_t face) const;		
	protected:

		TVector<RHIRenderTargetPtr> m_faces;

#if defined(SAILOR_BUILD_WITH_VULKAN)
		friend class Sailor::GraphicsDriver::Vulkan::VulkanGraphicsDriver;
#endif
	};
};
