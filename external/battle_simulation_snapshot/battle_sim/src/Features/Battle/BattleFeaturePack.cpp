// Battle feature-pack skeleton.

#include "Features/Battle/BattleFeaturePack.hpp"

#include "Core/Engine/GameContext.hpp"
#include "Features/Battle/Conditions/BattleConditionHandlers.hpp"
#include "Features/Battle/Effects/BattleEffects.hpp"
#include "Features/Battle/Policies/BattlePolicyAccess.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"
#include "Features/Battle/Selectors/BattleTargetSelectors.hpp"
#include "Features/Battle/Systems/BattleTurnSystem.hpp"
#include "Features/Battle/Values/BattleValueResolvers.hpp"

#include <functional>
#include <memory>
#include <utility>

namespace battle_sim::features::battle
{
	namespace
	{
		class ScheduledFunction final : public core::engine::IScheduledSystem
		{
		public:
			explicit ScheduledFunction(std::function<bool(core::engine::GameContext&)> run)
				: _run(std::move(run))
			{
			}

			bool run(core::engine::GameContext& context) override
			{
				return _run(context);
			}

		private:
			std::function<bool(core::engine::GameContext&)> _run;
		};

		std::unique_ptr<core::engine::IScheduledSystem> scheduledFunction(
			std::function<bool(core::engine::GameContext&)> run)
		{
			return std::make_unique<ScheduledFunction>(std::move(run));
		}

		void registerBattleTurnSchedule(core::engine::GameContext& context)
		{
			context.resources.emplace<BattleTurnRuntimeState>(BattleTurnRuntimeState{});

			context.scheduler.add(
				core::engine::Phase::Startup,
				scheduledFunction(
					[](core::engine::GameContext& context)
					{
						if (context.resources.contains<BattleMapResource>())
						{
							context.world.setTick(2);
						}
						return false;
					}));

			context.scheduler.add(
				core::engine::Phase::BeforeTick,
				scheduledFunction(
					[](core::engine::GameContext& context) { return !BattleTurnSystem::canContinue(context); }));

			context.scheduler.add(
				core::engine::Phase::Tick,
				scheduledFunction(
					[](core::engine::GameContext& context)
					{
						auto& state = context.resources.get<BattleTurnRuntimeState>();
						state.lastTickActions = BattleTurnSystem::executeTick(context);
						state.actionsExecuted += state.lastTickActions;
						return false;
					}));

			context.scheduler.add(
				core::engine::Phase::AfterTick,
				scheduledFunction(
					[](core::engine::GameContext& context)
					{
						auto& state = context.resources.get<BattleTurnRuntimeState>();
						BattleTurnSystem::cleanupDead(context);
						return shouldStopAfterNoActions(battlePolicies(context), state.lastTickActions);
					}));
		}
	}

	core::registry::HandlerId BattleFeaturePack::id() const
	{
		return core::registry::HandlerId::fromString("battle.basic");
	}

	void BattleFeaturePack::registerFeature(core::engine::GameContext& context, const core::config::FeatureConfig&)
	{
		battlePolicies(context);
		registerBattleValueResolvers(context.registries);
		registerBattleConditionHandlers(context.registries);
		registerBattleTargetSelectors(context.registries);
		registerBattleEffects(context.registries);
		registerBattleTurnSchedule(context);
	}
}
