// Legacy scenario IO feature-pack shell.

#pragma once

#include "Core/Registry/FeaturePack.hpp"

namespace battle_sim::io::legacy
{
	class LegacyIoFeaturePack final : public core::registry::IFeaturePack
	{
	public:
		core::registry::HandlerId id() const override;
		void registerFeature(core::engine::GameContext& context, const core::config::FeatureConfig& config) override;
	};
}
