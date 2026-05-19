// Tests for the domain-neutral ECS world and feature-owned battle components.

#include "Core/Ecs/ComponentStore.hpp"
#include "Core/Ecs/Components.hpp"
#include "Core/Ecs/World.hpp"
#include "Core/Engine/EventBus.hpp"
#include "Core/Engine/GameContext.hpp"
#include "Core/IWorldEventSink.hpp"
#include "Core/Registry/CoreHandlers.hpp"
#include "Features/Battle/BattleFeaturePack.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"
#include "Features/Battle/Events/BattleEvents.hpp"
#include "Features/Battle/EntityRecipe.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"
#include "Features/Battle/Systems/BattleMarchSystem.hpp"
#include "Features/Battle/Systems/BattleSpawnSystem.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <any>
#include <vector>

namespace
{
	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	class CollectingSink final : public battle_sim::core::IWorldEventSink
	{
	public:
		void onEvent(const std::any& event) override
		{
			events.push_back(event);
		}

		std::vector<std::any> events;
	};

	battle_sim::core::engine::GameContext makeBattleContext()
	{
		battle_sim::core::engine::GameContext game;
		battle_sim::core::registry::registerCoreHandlers(game.registries);
		battle_sim::features::battle::BattleFeaturePack{}.registerFeature(
			game,
			battle_sim::core::config::FeatureConfig{});
		return game;
	}

	battle_sim::features::battle::EntityRecipe basicRecipe(
		battle_sim::core::EntityId id,
		battle_sim::core::Position position)
	{
		battle_sim::features::battle::EntityRecipe recipe;
		recipe.id = id;
		recipe.position = position;
		recipe.stats.health.hp = 10;
		recipe.stats.strength.value = 3;
		recipe.actionBudget.pointsPerTurn = 1;
		return recipe;
	}

	void componentStoreAddsGetsOverwritesAndRemovesFeatureComponents()
	{
		battle_sim::core::ecs::ComponentStore<battle_sim::features::battle::HealthComponent> health;
		const battle_sim::core::EntityId entity = 7;

		require(!health.has(entity), "Expected component to be absent initially");
		health.emplace(entity, battle_sim::features::battle::HealthComponent{5});
		require(health.has(entity), "Expected component to be present after emplace");
		require(health.get(entity)->hp == 5, "Expected component value to be readable");

		health.emplace(entity, battle_sim::features::battle::HealthComponent{9});
		require(health.size() == 1, "Expected overwrite not to duplicate component");
		require(health.get(entity)->hp == 9, "Expected component value to be overwritten");

		health.remove(entity);
		require(!health.has(entity), "Expected component to be removed");
	}

	void worldProvidesTypedFeatureComponentStoresWithoutCoreAliases()
	{
		battle_sim::core::ecs::World world;
		require(world.createEntity(7), "Expected entity creation");

		auto& health = world.components<battle_sim::features::battle::HealthComponent>();
		health.emplace(7, battle_sim::features::battle::HealthComponent{12});
		require(health.get(7)->hp == 12, "Expected feature-owned health component");

		auto& position = world.components<battle_sim::features::battle::PositionComponent>();
		position.emplace(7, battle_sim::features::battle::PositionComponent{{3, 4}});
		require(position.get(7)->value == battle_sim::core::Position{3, 4}, "Expected feature-owned position component");

		world.removeEntity(7);
		require(!world.exists(7), "Expected entity removal");
		require(!health.has(7), "Expected feature health component removal with entity");
		require(!position.has(7), "Expected feature position component removal with entity");
	}

	void eventBusStoresAndMirrorsWorldEvents()
	{
		battle_sim::core::engine::EventBus events;
		CollectingSink sink;
		events.mirrorTo(&sink);

		events.publish(battle_sim::features::battle::EntityCreatedEvent{1, 7, {2, 3}});
		events.publish(battle_sim::features::battle::EntityRemovedEvent{3, 7, battle_sim::features::battle::RemovalReason::Death});

		require(events.events().size() == 2, "Expected stored events");
		require(sink.events.size() == 2, "Expected mirrored events");
		require(
			std::any_cast<battle_sim::features::battle::EntityCreatedEvent>(&events.events().front()) != nullptr,
			"Expected entity-created event");
	}

	void battleSpawnAndMarchUseFeatureOwnedComponents()
	{
		auto game = makeBattleContext();
		game.resources.emplace<battle_sim::features::battle::BattleMapResource>(3, 3);

		require(
			battle_sim::features::battle::BattleSpawnSystem::spawn(basicRecipe(1, {0, 0}), game, 1),
			"Expected battle spawn");
		require(
			game.world.components<battle_sim::features::battle::PositionComponent>().get(1)->value
				== battle_sim::core::Position{0, 0},
			"Expected battle position component");
		require(
			game.world.components<battle_sim::features::battle::HealthComponent>().get(1)->hp == 10,
			"Expected battle health component");

		require(
			battle_sim::features::battle::BattleMarchSystem::start(1, {2, 2}, game, 1),
			"Expected march command");
		require(
			game.world.components<battle_sim::features::battle::MarchTargetComponent>().get(1)->target
				== battle_sim::core::Position{2, 2},
			"Expected battle march target component");
	}
}

int main()
{
	const std::vector<std::pair<std::string, void (*)()>> tests{
		{"componentStoreAddsGetsOverwritesAndRemovesFeatureComponents", componentStoreAddsGetsOverwritesAndRemovesFeatureComponents},
		{"worldProvidesTypedFeatureComponentStoresWithoutCoreAliases", worldProvidesTypedFeatureComponentStoresWithoutCoreAliases},
		{"eventBusStoresAndMirrorsWorldEvents", eventBusStoresAndMirrorsWorldEvents},
		{"battleSpawnAndMarchUseFeatureOwnedComponents", battleSpawnAndMarchUseFeatureOwnedComponents},
	};

	for (const auto& [name, test] : tests)
	{
		try
		{
			test();
		}
		catch (const std::exception& error)
		{
			std::cerr << name << " failed: " << error.what() << '\n';
			return 1;
		}
	}

	std::cout << "ecs_world_tests: " << tests.size() << " passed\n";
	return 0;
}
