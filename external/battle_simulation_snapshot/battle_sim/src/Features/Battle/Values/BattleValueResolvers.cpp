// Battle-owned numeric value resolvers.

#include "Features/Battle/Values/BattleValueResolvers.hpp"

#include "Core/Engine/ActionContext.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"

#include <memory>

namespace battle_sim::features::battle
{
	namespace
	{
		class StrengthValueResolver final : public core::registry::IValueResolver
		{
		public:
			int resolve(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule&) const override
			{
				if (const auto* value = context.world().components<StrengthComponent>().get(actor))
				{
					return value->value;
				}
				return 0;
			}
		};

		class AgilityValueResolver final : public core::registry::IValueResolver
		{
		public:
			int resolve(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule&) const override
			{
				if (const auto* value = context.world().components<AgilityComponent>().get(actor))
				{
					return value->value;
				}
				return 0;
			}
		};

		class RangeValueResolver final : public core::registry::IValueResolver
		{
		public:
			int resolve(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule&) const override
			{
				if (const auto* value = context.world().components<RangeComponent>().get(actor))
				{
					return value->value;
				}
				return 0;
			}
		};

		class SpiritValueResolver final : public core::registry::IValueResolver
		{
		public:
			int resolve(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule&) const override
			{
				if (const auto* value = context.world().components<SpiritComponent>().get(actor))
				{
					return value->value;
				}
				return 0;
			}
		};

		class PowerValueResolver final : public core::registry::IValueResolver
		{
		public:
			int resolve(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule&) const override
			{
				if (const auto* value = context.world().components<PowerComponent>().get(actor))
				{
					return value->value;
				}
				return 0;
			}
		};

		class MobileStepValueResolver final : public core::registry::IValueResolver
		{
		public:
			int resolve(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule& rule) const override
			{
				if (const auto* value = context.world().components<MobileComponent>().get(actor))
				{
					return value->stepDistance;
				}
				return rule.stepDistance > 0 ? rule.stepDistance : 1;
			}
		};
	}

	void registerBattleValueResolvers(core::registry::RegistryHub& registries)
	{
		registries.valueResolvers.add("battle.strength", std::make_unique<StrengthValueResolver>());
		registries.valueResolvers.add("battle.agility", std::make_unique<AgilityValueResolver>());
		registries.valueResolvers.add("battle.range", std::make_unique<RangeValueResolver>());
		registries.valueResolvers.add("battle.spirit", std::make_unique<SpiritValueResolver>());
		registries.valueResolvers.add("battle.power", std::make_unique<PowerValueResolver>());
		registries.valueResolvers.add("battle.mobile-step", std::make_unique<MobileStepValueResolver>());
	}
}
