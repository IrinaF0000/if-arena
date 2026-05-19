#include "Features/UnitsClassic/UnitsClassicFeaturePack.hpp"

#include "Features/Battle/EntityArchetypeRegistry.hpp"
#include "Core/Engine/GameContext.hpp"
#include "Features/Battle/Policies/BattlePolicies.hpp"
#include "Features/UnitsClassic/ClassicUnitArchetypes.hpp"

#include <stdexcept>

namespace battle_sim::features::units_classic
{
	core::registry::HandlerId UnitsClassicFeaturePack::id() const
	{
		return core::registry::HandlerId::fromString("units.classic");
	}

	void UnitsClassicFeaturePack::registerFeature(core::engine::GameContext& context, const core::config::FeatureConfig&)
	{
		if (!context.resources.contains<battle_sim::features::battle::BattlePolicies>()
			|| !context.registries.effects.contains("battle.damage"))
		{
			throw std::runtime_error("units.classic requires battle.basic");
		}

		auto& archetypes = context.resources.contains<features::battle::EntityArchetypeRegistry>()
			? context.resources.get<features::battle::EntityArchetypeRegistry>()
			: context.resources.emplace<features::battle::EntityArchetypeRegistry>();
		registerClassicUnitArchetypes(archetypes);
	}
}
