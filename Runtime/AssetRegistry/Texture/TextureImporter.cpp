#include "AssetRegistry/Texture/TextureImporter.h"
#include "AssetRegistry/UID.h"
#include "AssetRegistry/AssetRegistry.h"
#include "TextureAssetInfo.h"
#include "Core/Utils.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>
#include "nlohmann_json/include/nlohmann/json.hpp"
#include "JobSystem/JobSystem.h"
#include "RHI/Texture.h"
#include "RHI/Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

using namespace Sailor;

bool Texture::IsReady() const
{
	return m_rhiTexture && m_rhiTexture->IsReady();
}

TextureImporter::TextureImporter(TextureAssetInfoHandler* infoHandler)
{
	SAILOR_PROFILE_FUNCTION();

	infoHandler->Subscribe(this);
}

TextureImporter::~TextureImporter()
{
}

void TextureImporter::OnUpdateAssetInfo(AssetInfoPtr assetInfo, bool bWasExpired)
{
}


void TextureImporter::OnImportAsset(AssetInfoPtr assetInfo)
{
}

bool TextureImporter::IsTextureLoaded(UID uid) const
{
	return m_loadedTextures.ContainsKey(uid);
}

bool TextureImporter::ImportTexture(UID uid, ByteCode& decodedData, int32_t& width, int32_t& height, uint32_t& mipLevels)
{
	SAILOR_PROFILE_FUNCTION();

	if (TextureAssetInfoPtr assetInfo = App::GetSubmodule<AssetRegistry>()->GetAssetInfoPtr<TextureAssetInfoPtr>(uid))
	{
		int32_t texChannels = 0;

		stbi_uc* pixels = stbi_load(assetInfo->GetAssetFilepath().c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
		uint32_t imageSize = (uint32_t)width * height * 4;
		decodedData.Resize(imageSize);
		memcpy(decodedData.GetData(), pixels, imageSize);

		mipLevels = assetInfo->ShouldGenerateMips() ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;
		stbi_image_free(pixels);
		return true;
	}

	return false;
}

bool TextureImporter::LoadTexture_Immediate(UID uid, TexturePtr& outTexture)
{
	auto task = LoadTexture(uid, outTexture);
	task->Wait();
	return task->GetResult();
}

JobSystem::TaskPtr<bool> TextureImporter::LoadTexture(UID uid, TexturePtr& outTexture)
{
	SAILOR_PROFILE_FUNCTION();

	JobSystem::TaskPtr<bool> newPromise;
	outTexture = nullptr;

	// Check promises first
	auto it = m_promises.Find(uid);
	if (it != m_promises.end())
	{
		newPromise = (*it).m_second;
	}

	// Check loaded textures
	auto textureIt = m_loadedTextures.Find(uid);
	if (textureIt != m_loadedTextures.end())
	{
		outTexture = (*textureIt).m_second;
		if (newPromise != nullptr)
		{
			if (!newPromise)
			{
				return JobSystem::TaskPtr<bool>::Make(true);
			}

			return newPromise;
		}
	}

	auto& promise = m_promises.At_Lock(uid, nullptr);

	// We have promise
	if (promise)
	{
		m_promises.Unlock(uid);
		outTexture = m_loadedTextures[uid];
		return promise;
	}

	if (TextureAssetInfoPtr assetInfo = App::GetSubmodule<AssetRegistry>()->GetAssetInfoPtr<TextureAssetInfoPtr>(uid))
	{
		auto pTexture = TSharedPtr<Texture>::Make(uid);

		newPromise = JobSystem::Scheduler::CreateTaskWithResult<bool>("Load model",
			[pTexture, assetInfo, this]()
			{
				ByteCode decodedData;
				int32_t width;
				int32_t height;
				uint32_t mipLevels;

				if (ImportTexture(assetInfo->GetUID(), decodedData, width, height, mipLevels))
				{
					pTexture.GetRawPtr()->m_rhiTexture = RHI::Renderer::GetDriver()->CreateImage(&decodedData[0], decodedData.Num(), glm::vec3(width, height, 1.0f),
						mipLevels, RHI::ETextureType::Texture2D, RHI::ETextureFormat::R8G8B8A8_SRGB, assetInfo->GetFiltration(),
						assetInfo->GetClamping());
					return true;
				}
				return false;
			});

		App::GetSubmodule<JobSystem::Scheduler>()->Run(newPromise);

		outTexture = m_loadedTextures[uid] = pTexture;
		promise = newPromise;
		m_promises.Unlock(uid);

		return promise;
	}

	m_promises.Unlock(uid);

	SAILOR_LOG("Cannot find texture with uid: %s", uid.ToString().c_str());
	return JobSystem::TaskPtr<bool>::Make(false);
}