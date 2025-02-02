#pragma once
#include <array>
#include "Engine/World.h"
#include "Engine/Types.h"
#include "Core/Submodule.h"
#include "Memory/UniquePtr.hpp"
#include "Memory/SharedPtr.hpp"
#include "Frame.h"

namespace Sailor
{
	class EngineLoop : public TSubmodule<EngineLoop>
	{
	public:

		SAILOR_API void ProcessCpuFrame(FrameState& currentInputState);
		SAILOR_API uint32_t GetCpuFps() const { return m_cpuFps; }

		EngineLoop() = default;
		SAILOR_API ~EngineLoop() override;

		SAILOR_API TSharedPtr<World> CreateWorld(std::string name);

	protected:

		uint32_t m_cpuFps = 0u;
		TVector<TSharedPtr<World>> m_worlds;
	};
}