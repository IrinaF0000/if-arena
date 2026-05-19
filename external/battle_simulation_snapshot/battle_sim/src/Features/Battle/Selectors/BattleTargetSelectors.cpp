// Battle-owned target selectors.

#include "Features/Battle/Selectors/BattleTargetSelectors.hpp"

#include "Core/Engine/ActionContext.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"
#include "Features/Battle/Selectors/SpatialQuery.hpp"

#include <memory>
#include <vector>

namespace battle_sim::features::battle
{
	namespace
	{
		int maxDistance(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule& rule)
		{
			if (rule.maxDistanceValueResolver.has_value()
				&& rule.maxDistanceValueResolver->str().rfind("battle.", 0) == 0
				&& context.registries().valueResolvers.contains(*rule.maxDistanceValueResolver))
			{
				return context.registries().valueResolvers.get(*rule.maxDistanceValueResolver).resolve(context, actor, rule);
			}
			return rule.maxDistance;
		}

		class AdjacentBlockingSelector final : public core::registry::ITargetSelector
		{
		public:
			std::vector<core::ecs::EntityId> select(
				core::engine::ActionContext& context,
				core::ecs::EntityId actor,
				const core::ecs::ActionRule&) const override
			{
				return SpatialQuery(context.world()).adjacentBlocking(actor);
			}
		};

		class EffectiveRangeSelector final : public core::registry::ITargetSelector
		{
		public:
			std::vector<core::ecs::EntityId> select(
				core::engine::ActionContext& context,
				core::ecs::EntityId actor,
				const core::ecs::ActionRule& rule) const override
			{
				const int maxDist = maxDistance(context, actor, rule);
				return SpatialQuery(context.world()).effectiveRange(actor, rule.minDistance, maxDist);
			}
		};

		class PhysicalRadiusSelector final : public core::registry::ITargetSelector
		{
		public:
			std::vector<core::ecs::EntityId> select(
				core::engine::ActionContext& context,
				core::ecs::EntityId actor,
				const core::ecs::ActionRule& rule) const override
			{
				const int maxDist = maxDistance(context, actor, rule);
				return SpatialQuery(context.world()).physicalRadius(actor, maxDist);
			}
		};
	}

	void registerBattleTargetSelectors(core::registry::RegistryHub& registries)
	{
		registries.targetSelectors.add("battle.adjacent-blocking", std::make_unique<AdjacentBlockingSelector>());
		registries.targetSelectors.add("battle.effective-range", std::make_unique<EffectiveRangeSelector>());
		registries.targetSelectors.add("battle.physical-radius", std::make_unique<PhysicalRadiusSelector>());
	}
}
