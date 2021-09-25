#pragma once
#include "AssetInfo.h"
#include "RHI/RHIResource.h"
#include "Core/Singleton.hpp"

using namespace std;

namespace Sailor
{
	class TextureAssetInfo final : public AssetInfo
	{
	public:
		virtual SAILOR_API ~TextureAssetInfo() = default;

		virtual SAILOR_API void Serialize(nlohmann::json& outData) const override;
		virtual SAILOR_API void Deserialize(const nlohmann::json& inData) override;

		RHI::ETextureFiltration GetFiltration() const { return m_filtration; }
		RHI::ETextureClamping GetClamping() const { return m_clamping; }
		bool ShouldGenerateMips() const { return m_bShouldGenerateMips; }

	private:

		RHI::ETextureFiltration m_filtration = RHI::ETextureFiltration::Linear;
		RHI::ETextureClamping m_clamping = RHI::ETextureClamping::Repeat;
		bool m_bShouldGenerateMips = true;
	};

	class TextureAssetInfoHandler final : public TSingleton<TextureAssetInfoHandler>, public IAssetInfoHandler
	{

	public:

		static SAILOR_API void Initialize();

		virtual SAILOR_API void GetDefaultMetaJson(nlohmann::json& outDefaultJson) const;
		virtual SAILOR_API AssetInfo* CreateAssetInfo() const;

		virtual SAILOR_API ~TextureAssetInfoHandler() = default;
	};
}
