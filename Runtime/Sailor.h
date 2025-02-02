#pragma once
#include "Core/Defines.h"
#include "Core/LogMacros.h"
#include "Core/Utils.h"
#include "Core/Submodule.h"
#include "Memory/SharedPtr.hpp"
#include "Memory/WeakPtr.hpp"
#include "Memory/UniquePtr.hpp"
#include "Platform/Win32/Window.h"
#include "Containers/Containers.h"
#include <glm/glm/glm.hpp>

namespace Sailor
{
	class AssetRegistry;

	struct AppArgs
	{
		bool m_bWaitForDebugger = false;
		bool m_bRunConsole = true;
		bool m_bIsEditor = false;
		uint32_t m_editorPort = 32800;
		HWND m_editorHwnd{};
	};

	class SAILOR_API App
	{
		static constexpr size_t MaxSubmodules = 64u;

	public:

		static void Initialize(const char** commandLineArgs = nullptr, int32_t num = 0);
		static void Start();
		static void Stop();
		static void Shutdown();

		static SubmoduleBase* GetSubmodule(uint32_t index) { return s_pInstance->m_submodules[index].GetRawPtr(); }

		template<typename T>
		static T* GetSubmodule()
		{
			const int32_t typeId = TSubmodule<T>::GetTypeId();
			if (typeId != SubmoduleBase::InvalidSubmoduleTypeId)
			{
				return s_pInstance->m_submodules[(uint32_t)typeId].StaticCast<T>();
			}
			return nullptr;
		}

		template<typename T>
		static T* AddSubmodule(TUniquePtr<TSubmodule<T>>&& submodule)
		{
			check(submodule);
			check(TSubmodule<T>::GetTypeId() != SubmoduleBase::InvalidSubmoduleTypeId);
			check(!s_pInstance->m_submodules[(uint32_t)TSubmodule<T>::GetTypeId()]);

			T* rawPtr = static_cast<T*>(submodule.GetRawPtr());
			s_pInstance->m_submodules[TSubmodule<T>::GetTypeId()] = std::move(submodule);

			return rawPtr;
		}

		template<typename T>
		static void RemoveSubmodule()
		{
			check(TSubmodule<T>::GetTypeId() != SubmoduleBase::InvalidSubmoduleTypeId);
			s_pInstance->m_submodules[TSubmodule<T>::GetTypeId()].Clear();
		}

		static TUniquePtr<Win32::Window>& GetMainWindow();
		static const char* GetApplicationName() { return "SailorEngine"; }
		static const char* GetEngineName() { return "SailorEngine"; }

	protected:

		TUniquePtr<Win32::Window> m_pMainWindow;

	private:

		TUniquePtr<SubmoduleBase> m_submodules[MaxSubmodules];

		static App* s_pInstance;

		App(const App&) = delete;
		App(App&&) = delete;

		App() = default;
		~App() = default;
	};
}