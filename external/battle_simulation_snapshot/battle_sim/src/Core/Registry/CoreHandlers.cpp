// Registers domain-neutral Core action handlers.

#include "Core/Registry/CoreHandlers.hpp"

#include "Core/Engine/ActionContext.hpp"

#include <cstddef>
#include <memory>
#include <random>
#include <vector>

namespace battle_sim::core::registry
{
	namespace
	{
		class AlwaysCondition final : public ICondition
		{
		public:
			bool matches(engine::ActionContext&, ecs::EntityId, const ecs::ActionRule&) const override
			{
				return true;
			}
		};

		class NoneSelector final : public ITargetSelector
		{
		public:
			std::vector<ecs::EntityId> select(engine::ActionContext&, ecs::EntityId, const ecs::ActionRule&) const override
			{
				return {};
			}
		};

		class NonePicker final : public ITargetPicker
		{
		public:
			std::vector<ecs::EntityId> pick(
				engine::ActionContext&,
				ecs::EntityId,
				const ecs::ActionRule&,
				const std::vector<ecs::EntityId>&) const override
			{
				return {};
			}
		};

		class AnyPicker final : public ITargetPicker
		{
		public:
			std::vector<ecs::EntityId> pick(
				engine::ActionContext&,
				ecs::EntityId,
				const ecs::ActionRule&,
				const std::vector<ecs::EntityId>& candidates) const override
			{
				if (candidates.empty())
				{
					return {};
				}
				return {candidates.front()};
			}
		};

		class AllPicker final : public ITargetPicker
		{
		public:
			std::vector<ecs::EntityId> pick(
				engine::ActionContext&,
				ecs::EntityId,
				const ecs::ActionRule&,
				const std::vector<ecs::EntityId>& candidates) const override
			{
				return candidates;
			}
		};

		class RandomOnePicker final : public ITargetPicker
		{
		public:
			std::vector<ecs::EntityId> pick(
				engine::ActionContext& context,
				ecs::EntityId,
				const ecs::ActionRule&,
				const std::vector<ecs::EntityId>& candidates) const override
			{
				if (candidates.empty())
				{
					return {};
				}

				std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
				return {candidates[pick(context.rng())]};
			}
		};

		class LiteralValueResolver final : public IValueResolver
		{
		public:
			int resolve(engine::ActionContext&, ecs::EntityId, const ecs::ActionRule& rule) const override
			{
				const auto it = rule.intParams.find("value");
				if (it == rule.intParams.end())
				{
					return 0;
				}
				return it->second;
			}
		};
	}

	void registerCoreHandlers(RegistryHub& registries)
	{
		registries.conditions.add("core.always", std::make_unique<AlwaysCondition>());
		registries.targetSelectors.add("core.none", std::make_unique<NoneSelector>());
		registries.targetPickers.add("core.none", std::make_unique<NonePicker>());
		registries.targetPickers.add("core.any", std::make_unique<AnyPicker>());
		registries.targetPickers.add("core.all", std::make_unique<AllPicker>());
		registries.targetPickers.add("core.random-one", std::make_unique<RandomOnePicker>());
		registries.valueResolvers.add("core.literal", std::make_unique<LiteralValueResolver>());
	}
}
