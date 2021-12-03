#pragma once
#include "Core/Defines.h"
#include <string>
#include <vector>
#include <nlohmann_json/include/nlohmann/json.hpp>
#include "Core/Submodule.h"
#include "Memory/SharedPtr.hpp"
#include "Memory/WeakPtr.hpp"
#include "AssetRegistry/AssetInfo.h"
#include "ShaderAssetInfo.h"
#include "RHI/Types.h"
#include "RHI/Renderer.h"
#include "ShaderCache.h"
#include "Framework/Object.h"

namespace Sailor
{
	class Shader : public Object
	{
	public:

		Shader(UID uid, std::vector<string> defines) : Object(uid), m_defines(defines) {}

		virtual bool IsReady() const override;

		const RHI::ShaderPtr& GetVertexShaderRHI() const { return m_rhiVertexShader; }
		RHI::ShaderPtr& GetVertexShaderRHI() { return m_rhiVertexShader; }

		const RHI::ShaderPtr& GetFragmentShaderRHI() const { return m_rhiFragmentShader; }
		RHI::ShaderPtr& GetFragmentShaderRHI() { return m_rhiFragmentShader; }

	protected:

		RHI::ShaderPtr m_rhiVertexShader;
		RHI::ShaderPtr m_rhiFragmentShader;
		std::vector<std::string> m_defines;

		friend class ShaderCompiler;
	};

	class ShaderAsset : IJsonSerializable
	{
	public:

		// Shader's code
		std::string m_glslVertex;
		std::string m_glslFragment;

		// Library code
		std::string m_glslCommon;

		std::vector<std::string> m_includes;
		std::vector<std::string> m_defines;

		SAILOR_API bool ContainsFragment() const { return !m_glslFragment.empty(); }
		SAILOR_API bool ContainsVertex() const { return !m_glslVertex.empty(); }
		SAILOR_API bool ContainsCommon() const { return !m_glslCommon.empty(); }

		virtual SAILOR_API void Serialize(nlohmann::json& outData) const;
		virtual SAILOR_API void Deserialize(const nlohmann::json& inData);
	};

	class ShaderCompiler final : public TSubmodule<ShaderCompiler>, public IAssetInfoHandlerListener
	{
	public:
		SAILOR_API ShaderCompiler(ShaderAssetInfoHandler* infoHandler);

		SAILOR_API void CompileAllPermutations(const UID& assetUID);
		SAILOR_API TWeakPtr<ShaderAsset> LoadShaderAsset(const UID& uid);

		SAILOR_API void GetSpirvCode(const UID& assetUID, const std::vector<std::string>& defines, RHI::ShaderByteCode& outVertexByteCode, RHI::ShaderByteCode& outFragmentByteCode, bool bIsDebug);

		virtual SAILOR_API ~ShaderCompiler() override;

		SAILOR_API bool CompileGlslToSpirv(const std::string& source, RHI::EShaderStage shaderKind, const std::vector<std::string>& defines, const std::vector<std::string>& includes, RHI::ShaderByteCode& outByteCode, bool bIsDebug);

		virtual SAILOR_API void OnImportAsset(AssetInfoPtr assetInfo) override;
		virtual SAILOR_API void OnUpdateAssetInfo(AssetInfoPtr assetInfo, bool bWasExpired) override;

	protected:

		ShaderCache m_shaderCache;
		std::unordered_map<UID, TSharedPtr<ShaderAsset>> m_loadedShaders;

		SAILOR_API void GeneratePrecompiledGlsl(ShaderAsset* shader, std::string& outGLSLCode, const std::vector<std::string>& defines = {});
		SAILOR_API void ConvertRawShaderToJson(const std::string& shaderText, std::string& outCodeInJSON);
		SAILOR_API bool ConvertFromJsonToGlslCode(const std::string& shaderText, std::string& outPureGLSL);

		SAILOR_API void ForceCompilePermutation(const UID& assetUID, uint32_t permutation);

		SAILOR_API uint32_t GetPermutation(const std::vector<std::string>& defines, const std::vector<std::string>& actualDefines);
		SAILOR_API std::vector<std::string> GetDefines(const std::vector<std::string>& defines, uint32_t permutation);

	private:

		static constexpr char const* JsonBeginCodeTag = "BEGIN_CODE";
		static constexpr char const* JsonEndCodeTag = "END_CODE";
		static constexpr char const* JsonEndLineTag = " END_LINE ";
	};
}
