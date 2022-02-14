#pragma once
#include "Core/Defines.h"
#include <string>
#include "Containers/Vector.h"
#include "Containers/ConcurrentMap.h"
#include <nlohmann_json/include/nlohmann/json.hpp>
#include "Core/Submodule.h"
#include "Memory/SharedPtr.hpp"
#include "Memory/WeakPtr.hpp"
#include "AssetRegistry/AssetInfo.h"
#include "RHI/Renderer.h"
#include "ModelAssetInfo.h"
#include "JobSystem/JobSystem.h"
#include "ModelAssetInfo.h"
#include "Engine/Object.h"
#include "Memory/ObjectPtr.hpp"
#include "Memory/ObjectAllocator.hpp"
#include "RHI/Mesh.h"
#include "RHI/Material.h"
#include "AssetRegistry/Material/MaterialImporter.h"

namespace Sailor::RHI
{
	class Vertex;
}

namespace Sailor
{
	using ModelPtr = TObjectPtr<class Model>;

	class Model : public Object
	{
	public:

		Model(UID uid, TVector<RHI::MeshPtr> meshes = {}, TVector<MaterialPtr> materials = {}) :
			Object(std::move(uid)),
			m_meshes(std::move(meshes)),
			m_materials(std::move(materials)) {}

		const TVector<RHI::MeshPtr>& GetMeshes() const { return m_meshes; }
		const TVector<MaterialPtr>& GetMaterials() const { return m_materials; }

		TVector<RHI::MeshPtr>& GetMeshes() { return m_meshes; }
		TVector<MaterialPtr>& GetMaterials() { return m_materials; }

		// Should be triggered after mesh/material changes
		void Flush();

		virtual bool IsReady() const override;
		virtual ~Model() = default;

	protected:

		TVector<RHI::MeshPtr> m_meshes;
		TVector<MaterialPtr> m_materials;

		std::atomic<bool> m_bIsReady;

		friend class ModelImporter;
	};

	class ModelImporter final : public TSubmodule<ModelImporter>, public IAssetInfoHandlerListener
	{
	public:

		SAILOR_API ModelImporter(ModelAssetInfoHandler* infoHandler);
		virtual SAILOR_API ~ModelImporter() override;

		virtual SAILOR_API void OnUpdateAssetInfo(AssetInfoPtr assetInfo, bool bWasExpired) override;
		virtual SAILOR_API void OnImportAsset(AssetInfoPtr assetInfo) override;

		SAILOR_API JobSystem::TaskPtr<bool> LoadModel(UID uid, ModelPtr& outModel);
		SAILOR_API bool LoadModel_Immediate(UID uid, ModelPtr& outModel);

	protected:

		TConcurrentMap<UID, JobSystem::TaskPtr<bool>> m_promises;
		TConcurrentMap<UID, ModelPtr> m_loadedModels;

		static SAILOR_API bool ImportObjModel(ModelAssetInfoPtr assetInfo,
			TVector<RHI::MeshPtr>& outMeshes,
			TVector<AssetInfoPtr>& outMaterialUIDs);

		SAILOR_API void GenerateMaterialAssets(ModelAssetInfoPtr assetInfo);

		ObjectAllocatorPtr m_allocator;
	};
}
