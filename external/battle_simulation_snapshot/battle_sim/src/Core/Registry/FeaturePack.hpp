// Domain-neutral extension point for statically registered feature packs.

#pragma once

#include "Core/Config/FeatureConfig.hpp"
#include "Core/Registry/HandlerId.hpp"

namespace battle_sim::core::engine
{
	struct GameContext;
}

namespace battle_sim::core::registry
{
	class IFeaturePack
	{
	public:
		virtual ~IFeaturePack() = default;

		virtual HandlerId id() const = 0;
		virtual void registerFeature(engine::GameContext& context, const config::FeatureConfig& config) = 0;
	};
}
