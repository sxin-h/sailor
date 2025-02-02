#pragma once
#include <string>
#include <fstream>
#include "Containers/Containers.h"
#include "AssetRegistry/FileId.h"
#include "Core/Submodule.h"
#include "AssetRegistry/AssetInfo.h"
#include "Core/Singleton.hpp"
#include "nlohmann_json/include/nlohmann/json.hpp"

namespace Sailor
{
	class AssetInfo;
	using AssetInfoPtr = AssetInfo*;

	enum class EAssetType;

	class AssetRegistry final : public TSubmodule<AssetRegistry>
	{
	public:

		static constexpr const char* ContentRootFolder = "../Content/";
		static constexpr const char* CacheRootFolder = "../Cache/";
		static constexpr const char* MetaFileExtension = "asset";

		SAILOR_API virtual ~AssetRegistry() override;

		template<typename TBinaryType, typename TFilepath>
		static bool ReadBinaryFile(const TFilepath& filename, TVector<TBinaryType>& buffer)
		{
			std::ifstream file(filename, std::ios::ate | std::ios::binary);
			file.unsetf(std::ios::skipws);

			if (!file.is_open())
			{
				return false;
			}

			size_t fileSize = (size_t)file.tellg();
			buffer.Clear();

			size_t mod = fileSize % sizeof(TBinaryType);
			size_t size = fileSize / sizeof(TBinaryType) + (mod ? 1 : 0);
			buffer.Resize(size);

			//buffer.resize(fileSize / sizeof(T));

			file.seekg(0, std::ios::beg);
			file.read(reinterpret_cast<char*>(buffer.GetData()), fileSize);

			file.close();
			return true;
		}

		template<typename TBinaryType, typename TFilepath>
		static void WriteBinaryFile(const TFilepath& filename, const TVector<TBinaryType>& buffer)
		{
			std::ofstream file(filename, std::ofstream::binary);
			file.write(reinterpret_cast<const char*>(buffer.GetData()), buffer.Num() * sizeof(buffer[0]));
			file.close();
		}

		SAILOR_API static bool ReadAllTextFile(const std::string& filename, std::string& text);

		SAILOR_API void ScanContentFolder();
		SAILOR_API void ScanFolder(const std::string& folderPath);
		SAILOR_API const FileId& LoadAsset(const std::string& filepath);
		SAILOR_API const FileId& GetOrLoadAsset(const std::string& filepath);

		template<typename TAssetInfoPtr = AssetInfoPtr>
		TAssetInfoPtr GetAssetInfoPtr(FileId uid) const
		{
			return 	dynamic_cast<TAssetInfoPtr>(GetAssetInfoPtr_Internal(uid));
		}

		template<typename TAssetInfoPtr = AssetInfoPtr>
		TAssetInfoPtr GetAssetInfoPtr(const std::string& assetFilepath) const
		{
			return 	dynamic_cast<TAssetInfoPtr>(GetAssetInfoPtr_Internal(assetFilepath));
		}

		template<class TAssetInfo>
		void GetAllAssetInfos(TVector<FileId>& outAssetInfos) const
		{
			outAssetInfos.Clear();
			for (const auto& assetInfo : m_loadedAssetInfo)
			{
				if (dynamic_cast<TAssetInfo*>(*assetInfo.m_second))
				{
					outAssetInfos.Add(assetInfo.m_first);
				}
			}
		}

		SAILOR_API bool RegisterAssetInfoHandler(const TVector<std::string>& supportedExtensions, class IAssetInfoHandler* pAssetInfoHandler);
		SAILOR_API static std::string GetMetaFilePath(const std::string& assetFilePath);

	protected:

		SAILOR_API AssetInfoPtr GetAssetInfoPtr_Internal(FileId uid) const;
		SAILOR_API AssetInfoPtr GetAssetInfoPtr_Internal(const std::string& assetFilepath) const;

		TMap<FileId, AssetInfoPtr> m_loadedAssetInfo;
		TMap<std::string, FileId> m_fileIds;
		TMap<std::string, class IAssetInfoHandler*> m_assetInfoHandlers;
	};
}
