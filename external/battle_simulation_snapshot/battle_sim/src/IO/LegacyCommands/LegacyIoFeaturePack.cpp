#include "IO/LegacyCommands/LegacyIoFeaturePack.hpp"

#include "Core/Engine/GameContext.hpp"

namespace battle_sim::io::legacy
{
	core::registry::HandlerId LegacyIoFeaturePack::id() const
	{
		return core::registry::HandlerId::fromString("io.legacy");
	}

	void LegacyIoFeaturePack::registerFeature(core::engine::GameContext&, const core::config::FeatureConfig&)
	{
	}
}
