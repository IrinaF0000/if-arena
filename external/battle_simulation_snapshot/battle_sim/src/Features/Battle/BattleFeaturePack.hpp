// Battle feature-pack skeleton.

#pragma once

#include "Core/Registry/FeaturePack.hpp"

namespace battle_sim::features::battle
{
	class BattleFeaturePack final : public core::registry::IFeaturePack
	{
	public:
		core::registry::HandlerId id() const override;
		void registerFeature(core::engine::GameContext& context, const core::config::FeatureConfig& config) override;
	};
}
