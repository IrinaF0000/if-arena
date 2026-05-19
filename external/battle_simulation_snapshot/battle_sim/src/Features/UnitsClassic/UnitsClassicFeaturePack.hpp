// Classic unit archetype feature-pack shell.

#pragma once

#include "Core/Registry/FeaturePack.hpp"

namespace battle_sim::features::units_classic
{
	class UnitsClassicFeaturePack final : public core::registry::IFeaturePack
	{
	public:
		core::registry::HandlerId id() const override;
		void registerFeature(core::engine::GameContext& context, const core::config::FeatureConfig& config) override;
	};
}
