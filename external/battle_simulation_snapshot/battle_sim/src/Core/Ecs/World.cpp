// World owns ECS entity lifecycle invariants.

#include "Core/Ecs/World.hpp"

namespace battle_sim::core::ecs
{
	WorldComponents::WorldComponents(ComponentStorage& storage)
		: creationOrder(storage.store<CreationOrderComponent>())
		, alive(storage.store<AliveComponent>())
		, removed(storage.store<RemovedComponent>())
		, actionBudget(storage.store<ActionBudgetComponent>())
		, actionRules(storage.store<ActionRulesComponent>())
		, actionState(storage.store<ActionStateComponent>())
	{
	}

	World::World()
		: _components(_componentStorage)
	{
	}

	EntityId World::createEntity()
	{
		while (_nextId == InvalidEntityId || _entityOrder.contains(_nextId))
		{
			++_nextId;
		}

		const EntityId id = _nextId++;
		createEntity(id);
		return id;
	}

	bool World::createEntity(EntityId id)
	{
		if (id == InvalidEntityId || _entityOrder.contains(id))
		{
			return false;
		}

		_entityOrder.emplace(id, _creationOrder.size());
		_activeEntities.insert(id);
		_creationOrder.push_back(id);

		if (id >= _nextId)
		{
			_nextId = id + 1;
		}
		return true;
	}

	bool World::exists(EntityId id) const
	{
		return _activeEntities.contains(id);
	}

	bool World::removeEntity(EntityId id)
	{
		const auto it = _activeEntities.find(id);
		if (it == _activeEntities.end())
		{
			return false;
		}

		_activeEntities.erase(it);
		removeComponents(id);
		return true;
	}

	std::uint64_t World::creationOrder(EntityId id) const
	{
		const auto it = _entityOrder.find(id);
		if (it == _entityOrder.end())
		{
			return 0;
		}
		return it->second;
	}

	void World::removeComponents(EntityId id)
	{
		_componentStorage.removeEntity(id);
	}
}
