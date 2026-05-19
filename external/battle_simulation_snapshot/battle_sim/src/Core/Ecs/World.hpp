// World owns ECS entity lifecycle invariants.

#pragma once

#include "Core/CommonTypes.hpp"
#include "Core/Ecs/ActionRules.hpp"
#include "Core/Ecs/ComponentStore.hpp"
#include "Core/Ecs/Components.hpp"
#include "Core/Ecs/Entity.hpp"

#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace battle_sim::core::ecs
{
	struct WorldComponents
	{
		explicit WorldComponents(ComponentStorage& storage);

		ComponentStore<CreationOrderComponent>& creationOrder;
		ComponentStore<AliveComponent>& alive;
		ComponentStore<RemovedComponent>& removed;
		ComponentStore<ActionBudgetComponent>& actionBudget;
		ComponentStore<ActionRulesComponent>& actionRules;
		ComponentStore<ActionStateComponent>& actionState;
	};

	class World
	{
	public:
		World();

		EntityId createEntity();
		bool createEntity(EntityId id);
		bool exists(EntityId id) const;
		bool removeEntity(EntityId id);

		WorldComponents& components()
		{
			return _components;
		}

		const WorldComponents& components() const
		{
			return _components;
		}

		template <class T>
		ComponentStore<T>& components()
		{
			return _componentStorage.store<T>();
		}

		template <class T>
		const ComponentStore<T>& components() const
		{
			return _componentStorage.store<T>();
		}

		std::size_t activeCount() const
		{
			return _activeEntities.size();
		}

		const std::vector<EntityId>& creationOrder() const
		{
			return _creationOrder;
		}

		std::uint64_t creationOrder(EntityId id) const;

		Tick tick() const
		{
			return _tick;
		}

		void setTick(Tick tick)
		{
			_tick = tick;
		}

		void advanceTick()
		{
			++_tick;
		}

	private:
		EntityId _nextId{1};
		Tick _tick{1};
		std::vector<EntityId> _creationOrder;
		std::unordered_map<EntityId, std::uint64_t> _entityOrder;
		std::unordered_set<EntityId> _activeEntities;
		ComponentStorage _componentStorage;
		WorldComponents _components;

		void removeComponents(EntityId id);
	};
}
