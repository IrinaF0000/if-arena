#include "BattleEngine.hpp"
#include "ArenaConfig.hpp"

#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	using namespace if_arena::battle_core;

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	template <typename Fn>
	void requireThrows(Fn&& fn, const std::string& message)
	{
		try
		{
			fn();
		}
		catch (const std::invalid_argument&)
		{
			return;
		}
		throw std::runtime_error(message);
	}

	MatchConfig onePlayerMatch()
	{
		MatchConfig config;
		config.width = 3;
		config.height = 3;
		config.maxTicks = 3;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{1, 1}, 100});
		return config;
	}

	MatchConfig objectiveRunMatch()
	{
		const auto arena = makeSmallObjectiveRunArenaConfig();
		MatchConfig config;
		config.width = arena.dimensions.width;
		config.height = arena.dimensions.height;
		config.obstacles = arena.obstacles;
		config.hazards = arena.hazards;
		config.bases = {
			BaseZoneConfig{ArenaTeam::Red, arena.redBase->center, arena.redBase->radius},
			BaseZoneConfig{ArenaTeam::Blue, arena.blueBase->center, arena.blueBase->radius},
		};
		config.objective = ObjectiveConfig{*arena.objectiveSpawn, 0.75, 0.8, 10, 20, 1};
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Red, arena.redSpawn->cell, 100});
		config.players.push_back(PlayerConfig{PlayerId{2}, ArenaTeam::Blue, arena.blueSpawn->cell, 100});
		return config;
	}

	MatchConfig objectiveRulesMatch(std::uint32_t scoreLimit = 1, double carrierSpeedMultiplier = 1.0)
	{
		MatchConfig config;
		config.width = 21;
		config.height = 13;
		config.playerSpeedPerTick = 1.0;
		config.bases = {
			BaseZoneConfig{ArenaTeam::Red, Vec2i{10, 4}, 0.5},
			BaseZoneConfig{ArenaTeam::Blue, Vec2i{10, 8}, 0.5},
		};
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{10, 6}, 100});
		config.objective = ObjectiveConfig{Vec2i{10, 6}, 0.75, carrierSpeedMultiplier, 2, 1, scoreLimit};
		return config;
	}

	MatchConfig combatMatch()
	{
		MatchConfig config;
		config.width = 7;
		config.height = 5;
		config.combat = CombatConfig{40, 2.0, 2, 3.0, 2};
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{1, 2}, 100});
		config.players.push_back(PlayerConfig{PlayerId{2}, ArenaTeam::Red, Vec2i{3, 2}, 100});
		return config;
	}

	bool hasEvent(const std::vector<BattleEvent>& events, BattleEventType type)
	{
		for (const auto& event : events)
		{
			if (event.type == type)
			{
				return true;
			}
		}
		return false;
	}

	bool hasCell(const std::vector<Vec2i>& cells, Vec2i cell)
	{
		return std::find(cells.begin(), cells.end(), cell) != cells.end();
	}

	bool hasHazardAt(const std::vector<HazardConfig>& hazards, HazardKind kind, Vec2i cell)
	{
		return std::find_if(hazards.begin(), hazards.end(), [&](const HazardConfig& hazard) {
			return hazard.kind == kind && hazard.position == cell;
		}) != hazards.end();
	}

	bool canReach(const ArenaConfig& arena, Vec2i start, Vec2i goal)
	{
		if (!arena.objectiveSpawn.has_value() || hasCell(arena.obstacles, start) || hasCell(arena.obstacles, goal))
		{
			return false;
		}
		const auto inside = [&arena](Vec2i cell) {
			return cell.x >= 0 && cell.y >= 0 && cell.x < arena.dimensions.width && cell.y < arena.dimensions.height;
		};
		const std::array<Direction, 4> directions{Direction{1, 0}, Direction{-1, 0}, Direction{0, 1}, Direction{0, -1}};
		std::vector<Vec2i> frontier{start};
		std::vector<Vec2i> visited{start};
		for (std::size_t index = 0; index < frontier.size(); ++index)
		{
			const auto current = frontier[index];
			if (current == goal)
			{
				return true;
			}
			for (const auto direction : directions)
			{
				const Vec2i next{current.x + direction.dx, current.y + direction.dy};
				if (!inside(next) || hasCell(arena.obstacles, next) || hasCell(visited, next))
				{
					continue;
				}
				visited.push_back(next);
				frontier.push_back(next);
			}
		}
		return false;
	}

	bool routeViaCell(const ArenaConfig& arena, Vec2i start, Vec2i gate)
	{
		return arena.objectiveSpawn.has_value() && canReach(arena, start, gate) && canReach(arena, gate, *arena.objectiveSpawn);
	}

	void createsMatchSnapshot()
	{
		BattleEngine engine(onePlayerMatch());
		const auto snapshot = engine.snapshot();

		require(snapshot.tick == 0, "new match starts at tick zero");
		require(snapshot.width == 3, "snapshot exposes match width");
		require(snapshot.height == 3, "snapshot exposes match height");
		require(!snapshot.finished, "new match is not finished");
		require(snapshot.players.size() == 1, "new match has one configured player");
		require(snapshot.players.front().player == PlayerId{1}, "snapshot preserves player id");
		require(snapshot.players.front().position == Vec2i{1, 1}, "snapshot preserves spawn");
		require(snapshot.players.front().worldPosition == Vec2d{1.0, 1.0}, "snapshot exposes smooth world position");
		require(snapshot.players.front().team == ArenaTeam::Blue, "snapshot exposes player team");
	}

	void appliesDeterministicMoveIntent()
	{
		BattleEngine first(onePlayerMatch());
		BattleEngine second(onePlayerMatch());

		const auto firstResult = first.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		const auto secondResult = second.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		require(firstResult.accepted(), "first move command accepted");
		require(secondResult.accepted(), "second move command accepted");

		const auto firstEvents = first.tick();
		const auto secondEvents = second.tick();
		const auto firstSnapshot = first.snapshot();
		const auto secondSnapshot = second.snapshot();

		require(firstEvents.size() == secondEvents.size(), "event count is deterministic");
		require(firstEvents.front().type == BattleEventType::PlayerMoved, "first event is movement");
		require(firstEvents.front().from == Vec2i{1, 1}, "movement event records source");
		require(firstEvents.front().to == Vec2i{2, 1}, "movement event records destination");
		require(firstSnapshot.tick == secondSnapshot.tick, "tick is deterministic");
		require(firstSnapshot.players.front().position == secondSnapshot.players.front().position, "position is deterministic");
		require(firstSnapshot.players.front().position == Vec2i{2, 1}, "move intent updates position by one cell");
	}

	void rejectsInvalidClientAuthority()
	{
		BattleEngine engine(onePlayerMatch());

		const auto unknownPlayer = engine.submit(PlayerCommand::move(PlayerId{99}, Direction{1, 0}));
		require(!unknownPlayer.accepted(), "unknown player command rejected");

		const auto oversizedMove = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{2, 0}));
		require(!oversizedMove.accepted(), "oversized movement intent rejected");

		const auto invalidAttack = engine.submit(PlayerCommand::attack(PlayerId{1}, Direction{}));
		require(!invalidAttack.accepted(), "invalid attack aim rejected");

		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();
		require(events.size() == 1, "invalid commands do not emit gameplay events");
		require(snapshot.players.front().position == Vec2i{1, 1}, "invalid commands do not mutate position");
	}

	void movesSmoothlyUntilStopped()
	{
		auto config = onePlayerMatch();
		config.playerSpeedPerTick = 0.5;
		BattleEngine engine(config);

		const auto move = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		require(move.accepted(), "smooth move command accepted");
		engine.tick();
		const auto moving = engine.snapshot();
		require(moving.players.front().worldPosition == Vec2d{1.5, 1.0}, "smooth movement advances by configured speed");
		require(moving.players.front().desiredMovement == MovementVector{1.0, 0.0}, "move intent becomes desired movement");

		const auto stop = engine.submit(PlayerCommand::stop(PlayerId{1}));
		require(stop.accepted(), "stop command accepted");
		engine.tick();
		const auto stopped = engine.snapshot();
		require(stopped.players.front().worldPosition == Vec2d{1.5, 1.0}, "stop prevents further movement");
		require(stopped.players.front().desiredMovement == MovementVector{}, "stop clears desired movement");
	}

	void preventsLeavingArenaBounds()
	{
		MatchConfig config;
		config.width = 3;
		config.height = 3;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{0, 1}, 100});
		BattleEngine engine(config);

		const auto move = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{-1, 0}));
		require(move.accepted(), "boundary move intent accepted for server-side resolution");
		engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.players.front().position == Vec2i{0, 1}, "player cannot leave arena bounds");
		require(snapshot.players.front().worldPosition == Vec2d{0.0, 1.0}, "blocked boundary move does not change world position");
	}

	void preventsMovingThroughObstacles()
	{
		MatchConfig config;
		config.width = 5;
		config.height = 3;
		config.playerSpeedPerTick = 2.0;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{1, 1}, 100});
		config.obstacles.push_back(Vec2i{2, 1});
		BattleEngine engine(config);

		const auto move = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		require(move.accepted(), "obstacle move intent accepted for server-side resolution");
		engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.players.front().position == Vec2i{1, 1}, "player cannot move through obstacle");
		require(snapshot.players.front().worldPosition == Vec2d{1.0, 1.0}, "blocked obstacle move does not change world position");
	}

	void reportsBaseEntry()
	{
		MatchConfig config;
		config.width = 21;
		config.height = 13;
		config.bases.push_back(BaseZoneConfig{ArenaTeam::Blue, Vec2i{10, 11}, 0.5});
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{10, 10}, 100});
		BattleEngine engine(config);

		require(!engine.snapshot().players.front().inOwnBase, "player starts outside own base zone");
		const auto move = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{0, 1}));
		require(move.accepted(), "base entry movement accepted");
		engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.players.front().position == Vec2i{10, 11}, "player reaches own base cell");
		require(snapshot.players.front().inOwnBase, "player is marked inside own base");
	}

	void transformsPlayerOrientedCoordinates()
	{
		const auto dimensions = canonicalObjectiveRunDimensions();

		require(toPlayerView(Vec2i{10, 10}, ArenaTeam::Blue, dimensions) == Vec2i{10, 10},
		        "blue view keeps canonical bottom spawn");
		require(toPlayerView(Vec2i{10, 2}, ArenaTeam::Red, dimensions) == Vec2i{10, 10},
		        "red view rotates own top spawn to bottom");
		require(inputDirectionToWorld(Direction{0, -1}, ArenaTeam::Blue).dy == -1,
		        "blue forward input maps upward in canonical coordinates");
		require(inputDirectionToWorld(Direction{0, -1}, ArenaTeam::Red).dy == 1,
		        "red forward input maps downward in canonical coordinates");
		require(inputDirectionToWorld(Direction{1, 0}, ArenaTeam::Blue).dx == 1,
		        "blue right input maps right in canonical coordinates");
		require(inputDirectionToWorld(Direction{1, 0}, ArenaTeam::Red).dx == -1,
		        "red right input maps through the 180-degree local view");
	}

	void movesBlueSpawnDirectlyTowardObjective()
	{
		BattleEngine engine(objectiveRunMatch());

		require(engine.submit(PlayerCommand::move(PlayerId{2}, Direction{0, -1})).accepted(),
		        "blue forward movement accepted");
		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::PlayerMoved), "blue forward emits movement");
		require(snapshot.players.back().position == Vec2i{10, 9}, "blue moves one cell directly toward center");
		require(snapshot.players.back().worldPosition == Vec2d{10.0, 9.0}, "blue world movement advances one step");
	}

	void movesRedSpawnDirectlyTowardObjective()
	{
		BattleEngine engine(objectiveRunMatch());

		require(engine.submit(PlayerCommand::move(PlayerId{1}, Direction{0, 1})).accepted(),
		        "red forward canonical movement accepted");
		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::PlayerMoved), "red forward emits movement");
		require(snapshot.players.front().position == Vec2i{10, 3}, "red moves one cell directly toward center");
		require(snapshot.players.front().worldPosition == Vec2d{10.0, 3.0}, "red world movement advances one step");
	}

	void movesCarrierBackToOwnBaseAndCaptures()
	{
		BattleEngine engine(objectiveRulesMatch());

		require(engine.submit(PlayerCommand::move(PlayerId{1}, Direction{0, 1})).accepted(),
		        "carrier can move back toward own base without interact");
		auto events = engine.tick();
		require(hasEvent(events, BattleEventType::ObjectivePickedUp), "objective pickup is automatic on contact");
		events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::ObjectiveCaptured), "objective capture is automatic in own base");
		require(snapshot.finished, "score limit capture finishes the match");
		require(snapshot.scores.front().score == 1, "capture increments own team score");
	}

	void respectsConfiguredSmoothSpeedWithoutSkippingCells()
	{
		auto config = objectiveRunMatch();
		config.playerSpeedPerTick = 0.25;
		BattleEngine engine(config);

		require(engine.submit(PlayerCommand::move(PlayerId{2}, Direction{0, -1})).accepted(),
		        "playable smooth move accepted");
		engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.players.back().worldPosition == Vec2d{10.0, 9.75},
		        "smooth movement advances by configured cells per tick");
		require(snapshot.players.back().position == Vec2i{10, 10}, "first smooth tick does not skip a logical cell");
	}

	void completesObjectiveRunCaptureAndWin()
	{
		BattleEngine engine(objectiveRulesMatch());

		const auto pickup = engine.submit(PlayerCommand{PlayerId{1}, PlayerCommandType::Interact, {}});
		require(pickup.accepted(), "carrier can interact to pick up objective at spawn");
		auto events = engine.tick();
		require(hasEvent(events, BattleEventType::ObjectivePickedUp), "pickup event emitted");
		require(engine.snapshot().objective.state == ObjectiveState::Carried, "objective is carried after pickup");

		const auto move = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{0, 1}));
		require(move.accepted(), "carrier movement accepted");
		engine.tick();
		events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::ObjectiveCaptured), "capture event emitted at own base");
		require(hasEvent(events, BattleEventType::ScoreChanged), "score event emitted at capture");
		require(hasEvent(events, BattleEventType::MatchFinished), "match finish event emitted at score limit");
		require(snapshot.finished, "match finishes when score limit is reached");
		require(snapshot.scores.front().score == 1, "capture increments score");
		require(snapshot.objective.state == ObjectiveState::Captured, "objective records captured terminal state");
		require(!engine.dropObjective(PlayerId{1}).accepted(), "finished match rejects server-side objective drop");
	}

	void appliesCarrierSpeedMultiplier()
	{
		BattleEngine engine(objectiveRulesMatch(1, 0.5));
		require(engine.submit(PlayerCommand{PlayerId{1}, PlayerCommandType::Interact, {}}).accepted(), "pickup accepted");
		engine.tick();
		require(engine.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0})).accepted(), "carrier move accepted");
		engine.tick();

		const auto snapshot = engine.snapshot();
		require(snapshot.players.front().worldPosition == Vec2d{10.5, 6.0}, "carrier speed multiplier slows movement");
		require(snapshot.objective.position == Vec2d{10.5, 6.0}, "carried objective follows carrier");
	}

	void rejectsImmediateRepickupDuringLock()
	{
		BattleEngine engine(objectiveRulesMatch());
		require(engine.submit(PlayerCommand{PlayerId{1}, PlayerCommandType::Interact, {}}).accepted(), "pickup accepted");
		engine.tick();
		require(engine.dropObjective(PlayerId{1}).accepted(), "server-side drop accepted for carrier");

		const auto immediate = engine.submit(PlayerCommand{PlayerId{1}, PlayerCommandType::Interact, {}});
		require(!immediate.accepted(), "pickup lock rejects immediate re-pickup");
		const auto events = engine.tick();
		require(hasEvent(events, BattleEventType::ObjectiveDropped), "drop event emitted on next tick");
		require(engine.snapshot().objective.state == ObjectiveState::Dropped, "objective remains dropped during lock");
	}

	void failsCaptureAtEnemyBase()
	{
		MatchConfig config;
		config.width = 21;
		config.height = 13;
		config.bases = {
			BaseZoneConfig{ArenaTeam::Red, Vec2i{1, 1}, 0.5},
			BaseZoneConfig{ArenaTeam::Blue, Vec2i{10, 6}, 1.0},
		};
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Red, Vec2i{10, 6}, 100});
		config.objective = ObjectiveConfig{Vec2i{10, 6}, 0.75, 0.8, 1, 1, 1};
		BattleEngine engine(config);

		require(engine.submit(PlayerCommand{PlayerId{1}, PlayerCommandType::Interact, {}}).accepted(),
		        "red player can pick up objective at enemy base");
		engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.objective.state == ObjectiveState::Carried, "objective remains carried at enemy base");
		require(snapshot.scores.front().score == 0, "enemy base does not score");
		require(!snapshot.finished, "enemy base does not finish match");
	}

	void respawnsObjectiveAfterCaptureDelay()
	{
		BattleEngine engine(objectiveRulesMatch(2));
		require(engine.submit(PlayerCommand{PlayerId{1}, PlayerCommandType::Interact, {}}).accepted(), "pickup accepted");
		engine.tick();
		require(engine.submit(PlayerCommand::move(PlayerId{1}, Direction{0, 1})).accepted(), "carrier movement accepted");
		engine.tick();
		engine.tick();
		require(engine.snapshot().objective.state == ObjectiveState::Respawning, "objective waits to respawn after capture");

		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();
		require(hasEvent(events, BattleEventType::ObjectiveRespawned), "objective respawn event emitted");
		require(snapshot.objective.state == ObjectiveState::AtSpawn, "objective respawns at center after delay");
		require(!snapshot.finished, "match continues before score limit");
	}

	void appliesAttackHitAndCooldown()
	{
		BattleEngine engine(combatMatch());

		const auto attack = engine.submit(PlayerCommand::attack(PlayerId{1}, Direction{1, 0}));
		require(attack.accepted(), "valid attack intent accepted");
		const auto duplicate = engine.submit(PlayerCommand::attack(PlayerId{1}, Direction{1, 0}));
		require(!duplicate.accepted(), "duplicate same-tick attack rejected by cooldown policy");
		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::AttackHit), "attack hit event emitted");
		require(snapshot.players.back().hp == 60, "server applies attack damage");
		require(snapshot.players.front().attackCooldownTicksRemaining == 2, "attack cooldown is set by server");
	}

	void rejectsInvalidAttackAim()
	{
		BattleEngine engine(combatMatch());

		require(!engine.submit(PlayerCommand::attack(PlayerId{1}, Direction{})).accepted(), "zero attack aim rejected");
		require(!engine.submit(PlayerCommand::attack(PlayerId{1}, Direction{2, 0})).accepted(),
		        "oversized attack aim rejected");
	}

	void dashesWithoutLeavingArena()
	{
		MatchConfig config;
		config.width = 5;
		config.height = 3;
		config.combat = CombatConfig{20, 1.5, 2, 5.0, 2};
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{1, 1}, 100});
		BattleEngine engine(config);

		const auto dash = engine.submit(PlayerCommand::dash(PlayerId{1}, Direction{-1, 0}));
		require(dash.accepted(), "dash intent accepted");
		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::PlayerDashed), "dash event emitted");
		require(snapshot.players.front().worldPosition.x == 0.0, "dash clamps to arena edge");
		require(snapshot.players.front().dashCooldownTicksRemaining == 2, "dash cooldown is set by server");
	}

	void hazardsHitDeterministically()
	{
		MatchConfig config;
		config.width = 5;
		config.height = 5;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{2, 2}, 100});
		config.hazards.push_back(HazardConfig{HazardKind::Mine, Vec2i{2, 2}, 0.8, 1.0, 18, 3});
		BattleEngine engine(config);

		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::HazardTelegraphed), "hazard telegraph event emitted");
		require(hasEvent(events, BattleEventType::HazardHit), "hazard hit event emitted");
		require(snapshot.players.front().hp == 82, "hazard damage applied deterministically");
		require(snapshot.hazards.front().triggered, "mine records triggered state");
		require(snapshot.hazards.front().radius == 0.8, "hazard snapshot exposes configured radius");
		require(snapshot.hazards.front().range == 1.0, "hazard snapshot exposes configured range");
		require(snapshot.hazards.front().damage == 18, "hazard snapshot exposes configured damage");
		require(snapshot.hazards.front().cooldownTicks == 3, "hazard snapshot exposes configured cooldown duration");
		require(snapshot.hazards.front().effect == HazardEffect::DamageAndDropObjective,
		        "hazard snapshot exposes configured effect");
		require(snapshot.hazards.front().trigger == HazardTrigger::Proximity, "hazard snapshot exposes configured trigger");
		require(snapshot.hazards.front().icon == "hazard_mine", "hazard snapshot exposes configured icon");
	}

	void hazardDamageOnlyDoesNotDropObjective()
	{
		MatchConfig config;
		config.width = 5;
		config.height = 5;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{2, 2}, 100});
		config.objective = ObjectiveConfig{Vec2i{2, 2}, 0.75, 1.0, 2, 1, 1};
		config.hazards.push_back(HazardConfig{HazardKind::Mine, Vec2i{2, 2}, 0.8, 1.0, 10, 3, 0, "damage_only",
		                                      HazardEffect::Damage, HazardTrigger::Proximity, "hazard_mine"});
		BattleEngine engine(config);

		require(engine.submit(PlayerCommand{PlayerId{1}, PlayerCommandType::Interact, {}}).accepted(), "pickup accepted");
		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::HazardHit), "damage-only hazard still hits");
		require(!hasEvent(events, BattleEventType::ObjectiveDropped), "damage-only hazard does not drop objective");
		require(snapshot.players.front().hp == 90, "damage-only hazard applies configured damage");
		require(snapshot.objective.state == ObjectiveState::Carried, "objective remains carried after damage-only hazard");
	}

	void hazardTriggerUsesConfiguredRange()
	{
		MatchConfig config;
		config.width = 5;
		config.height = 5;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{2, 3}, 100});
		config.hazards.push_back(HazardConfig{HazardKind::Mine, Vec2i{2, 1}, 0.5, 2.1, 7, 3, 0, "range_mine",
		                                      HazardEffect::Damage, HazardTrigger::Range, "hazard_mine"});
		BattleEngine engine(config);

		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::HazardHit), "range-trigger hazard uses configured range");
		require(snapshot.players.front().hp == 93, "range-trigger hazard applies configured damage");
	}

	void crowHazardPatrolsAndPecksDeterministically()
	{
		MatchConfig firstConfig;
		firstConfig.width = 21;
		firstConfig.height = 13;
		firstConfig.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{11, 6}, 100});
		firstConfig.hazards.push_back(HazardConfig{HazardKind::Crow, Vec2i{10, 6}, 0.75, 1.5, 6, 2, 0});
		MatchConfig secondConfig = firstConfig;

		BattleEngine first(firstConfig);
		BattleEngine second(secondConfig);

		require(first.snapshot().hazards.front().position == Vec2i{11, 5}, "seeded crow starts on deterministic patrol cell");
		const auto firstEvents = first.tick();
		const auto secondEvents = second.tick();
		const auto firstSnapshot = first.snapshot();
		const auto secondSnapshot = second.snapshot();

		require(hasEvent(firstEvents, BattleEventType::HazardHit), "crow peck emits hazard hit");
		require(firstSnapshot.hazards.front().position == Vec2i{11, 6}, "crow moves along seeded center patrol");
		require(firstSnapshot.hazards.front().triggered, "crow records triggered peck state");
		require(firstSnapshot.players.front().hp == 94, "crow peck applies minor damage");
		require(firstSnapshot.hazards.front().position == secondSnapshot.hazards.front().position,
		        "crow patrol is deterministic across engines");
		require(firstSnapshot.players.front().hp == secondSnapshot.players.front().hp, "crow damage is deterministic");
	}

	void hitOnCarrierDropsObjective()
	{
		MatchConfig config;
		config.width = 21;
		config.height = 13;
		config.combat = CombatConfig{10, 2.0, 2, 2.0, 2};
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{10, 6}, 100});
		config.players.push_back(PlayerConfig{PlayerId{2}, ArenaTeam::Red, Vec2i{10, 5}, 100});
		config.objective = ObjectiveConfig{Vec2i{10, 6}, 0.75, 1.0, 2, 1, 1};
		BattleEngine engine(config);

		require(engine.submit(PlayerCommand{PlayerId{1}, PlayerCommandType::Interact, {}}).accepted(), "pickup accepted");
		engine.tick();
		require(engine.submit(PlayerCommand::attack(PlayerId{2}, Direction{0, 1})).accepted(), "opponent attack accepted");
		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(hasEvent(events, BattleEventType::AttackHit), "carrier hit event emitted");
		require(hasEvent(events, BattleEventType::ObjectiveDropped), "objective drops when carrier is hit");
		require(snapshot.objective.state == ObjectiveState::Dropped, "objective state becomes dropped after carrier hit");
		require(snapshot.objective.pickupLockTicksRemaining > 0, "drop starts pickup lock");
	}

	void defeatedPlayersCannotAct()
	{
		auto config = combatMatch();
		config.combat.attackDamage = 100;
		BattleEngine engine(config);

		require(engine.submit(PlayerCommand::attack(PlayerId{1}, Direction{1, 0})).accepted(), "finishing attack accepted");
		const auto events = engine.tick();
		require(hasEvent(events, BattleEventType::PlayerDefeated), "defeat event emitted");
		require(!engine.submit(PlayerCommand::attack(PlayerId{2}, Direction{-1, 0})).accepted(),
		        "defeated player cannot attack");
		require(!engine.submit(PlayerCommand::dash(PlayerId{2}, Direction{-1, 0})).accepted(),
		        "defeated player cannot dash");
	}

	void rejectsUnboundedCombatConfig()
	{
		MatchConfig config;
		config.width = 5;
		config.height = 5;
		config.combat.dashDistance = 500.0;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{2, 2}, 100});

		requireThrows([&config] { BattleEngine engine(config); }, "unbounded dash distance is rejected");
	}

	void finishesAtConfiguredTickLimit()
	{
		BattleEngine engine(onePlayerMatch());

		engine.tick();
		engine.tick();
		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.finished, "match finishes at max tick");
		require(events.back().type == BattleEventType::MatchFinished, "finish event is emitted");

		const auto afterFinish = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		require(!afterFinish.accepted(), "finished match rejects commands");
	}

	void acceptsCanonicalObjectiveRunArena()
	{
		const auto arena = makeSmallObjectiveRunArenaConfig();
		const auto validation = validateArenaConfig(arena);

		require(validation.valid(), "canonical 21x13 Objective Run arena is valid");
		require(arena.dimensions == canonicalObjectiveRunDimensions(), "canonical arena dimensions are 21x13");
		require(arena.objectiveSpawn == Vec2i{10, 6}, "canonical objective starts at center");
		require(arena.redBase.has_value() && arena.blueBase.has_value(), "canonical arena has both bases");
		require(arena.redSpawn.has_value() && arena.blueSpawn.has_value(), "canonical arena has both spawns");
		require(!arena.obstacles.empty(), "canonical arena has obstacle contest pressure");
		require(!arena.hazards.empty(), "canonical arena has hazard contest pressure");
		require(!hasCell(arena.obstacles, Vec2i{10, 9}), "blue direct lane from spawn is open");
		require(!hasCell(arena.obstacles, Vec2i{10, 3}), "red direct lane from spawn is open");
		require(hasCell(arena.obstacles, Vec2i{7, 5}) && hasCell(arena.obstacles, Vec2i{13, 7}),
		        "center wall shoulders keep rotational symmetry");
	}

	void canonicalObjectiveRunArenaHasThreeRoutesToObjective()
	{
		const auto arena = makeSmallObjectiveRunArenaConfig();
		require(arena.blueSpawn.has_value() && arena.redSpawn.has_value(), "canonical route test requires both spawns");

		const std::array<Vec2i, 3> blueGates{Vec2i{8, 7}, Vec2i{10, 7}, Vec2i{12, 7}};
		for (const auto gate : blueGates)
		{
			require(routeViaCell(arena, arena.blueSpawn->cell, gate), "blue has a distinct lower route to the objective");
			require(routeViaCell(arena, arena.redSpawn->cell, rotate180(gate, arena.dimensions)),
			        "red has the mirrored route to the objective");
		}

		require(hasHazardAt(arena.hazards, HazardKind::Mine, Vec2i{8, 6}), "left side route has mine pressure");
		require(hasHazardAt(arena.hazards, HazardKind::Mine, Vec2i{12, 6}), "right side route has mine pressure");
		require(hasHazardAt(arena.hazards, HazardKind::Crow, Vec2i{10, 6}), "center route has crow pressure");
	}

	void createsMatchFromCanonicalObjectiveRunArena()
	{
		BattleEngine engine(objectiveRunMatch());
		const auto snapshot = engine.snapshot();

		require(snapshot.width == 21, "canonical arena match exposes width");
		require(snapshot.height == 13, "canonical arena match exposes height");
		require(snapshot.players.size() == 2, "canonical arena match has both players");
		require(snapshot.players.front().team == ArenaTeam::Red, "red player keeps team assignment");
		require(snapshot.players.back().team == ArenaTeam::Blue, "blue player keeps team assignment");
		require(snapshot.players.front().spawn == Vec2i{10, 2}, "red spawn comes from canonical arena");
		require(snapshot.players.back().spawn == Vec2i{10, 10}, "blue spawn comes from canonical arena");
		require(snapshot.obstacles.size() == makeSmallObjectiveRunArenaConfig().obstacles.size(),
		        "canonical match exposes authoritative obstacles");
		require(hasCell(snapshot.obstacles, Vec2i{7, 5}), "canonical match exposes center wall shoulder");
		require(!snapshot.hazards.empty(), "canonical match exposes configured hazards");
		require(std::any_of(snapshot.hazards.begin(), snapshot.hazards.end(),
		                    [](const HazardSnapshot& hazard) { return hazard.kind == HazardKind::Crow; }),
		        "canonical match exposes the neutral crow hazard");
	}

	void rejectsAsymmetricObstacleLayout()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.obstacles.push_back(Vec2i{1, 1});

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "asymmetric obstacle layout is rejected");
	}

	void rejectsAsymmetricHazardLayout()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.hazards.erase(arena.hazards.begin());

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "asymmetric hazard layout is rejected");
	}

	void rejectsOverlappingArenaHazards()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.hazards.push_back(HazardConfig{HazardKind::Mine, arena.redBase->center, 0.7, 1.0, 12, 30});
		arena.hazards.push_back(arena.hazards.front());

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "overlapping hazard cells and base hazards are rejected");
	}

	void rejectsInvalidArenaDimensions()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.dimensions = ArenaDimensions{20, 13};

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "non-canonical arena dimensions are rejected");
	}

	void rejectsOutOfBoundsArenaObjects()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.redSpawn = ArenaSpawn{Vec2i{-1, 2}};
		arena.obstacles.push_back(Vec2i{21, 6});

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "out-of-bounds arena objects are rejected");
	}

	void rejectsMissingBases()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.redBase.reset();

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "missing base is rejected");
	}

	void rejectsInvalidObjectiveSpawn()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.objectiveSpawn = Vec2i{9, 6};

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "non-central objective spawn is rejected");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"createsMatchSnapshot", createsMatchSnapshot},
		{"appliesDeterministicMoveIntent", appliesDeterministicMoveIntent},
		{"rejectsInvalidClientAuthority", rejectsInvalidClientAuthority},
		{"movesSmoothlyUntilStopped", movesSmoothlyUntilStopped},
		{"preventsLeavingArenaBounds", preventsLeavingArenaBounds},
		{"preventsMovingThroughObstacles", preventsMovingThroughObstacles},
		{"reportsBaseEntry", reportsBaseEntry},
		{"transformsPlayerOrientedCoordinates", transformsPlayerOrientedCoordinates},
		{"movesBlueSpawnDirectlyTowardObjective", movesBlueSpawnDirectlyTowardObjective},
		{"movesRedSpawnDirectlyTowardObjective", movesRedSpawnDirectlyTowardObjective},
		{"movesCarrierBackToOwnBaseAndCaptures", movesCarrierBackToOwnBaseAndCaptures},
		{"respectsConfiguredSmoothSpeedWithoutSkippingCells", respectsConfiguredSmoothSpeedWithoutSkippingCells},
		{"completesObjectiveRunCaptureAndWin", completesObjectiveRunCaptureAndWin},
		{"appliesCarrierSpeedMultiplier", appliesCarrierSpeedMultiplier},
		{"rejectsImmediateRepickupDuringLock", rejectsImmediateRepickupDuringLock},
		{"failsCaptureAtEnemyBase", failsCaptureAtEnemyBase},
		{"respawnsObjectiveAfterCaptureDelay", respawnsObjectiveAfterCaptureDelay},
		{"appliesAttackHitAndCooldown", appliesAttackHitAndCooldown},
		{"rejectsInvalidAttackAim", rejectsInvalidAttackAim},
		{"dashesWithoutLeavingArena", dashesWithoutLeavingArena},
		{"hazardsHitDeterministically", hazardsHitDeterministically},
		{"hazardDamageOnlyDoesNotDropObjective", hazardDamageOnlyDoesNotDropObjective},
		{"hazardTriggerUsesConfiguredRange", hazardTriggerUsesConfiguredRange},
		{"crowHazardPatrolsAndPecksDeterministically", crowHazardPatrolsAndPecksDeterministically},
		{"hitOnCarrierDropsObjective", hitOnCarrierDropsObjective},
		{"defeatedPlayersCannotAct", defeatedPlayersCannotAct},
		{"rejectsUnboundedCombatConfig", rejectsUnboundedCombatConfig},
		{"finishesAtConfiguredTickLimit", finishesAtConfiguredTickLimit},
		{"acceptsCanonicalObjectiveRunArena", acceptsCanonicalObjectiveRunArena},
		{"canonicalObjectiveRunArenaHasThreeRoutesToObjective", canonicalObjectiveRunArenaHasThreeRoutesToObjective},
		{"createsMatchFromCanonicalObjectiveRunArena", createsMatchFromCanonicalObjectiveRunArena},
		{"rejectsAsymmetricObstacleLayout", rejectsAsymmetricObstacleLayout},
		{"rejectsAsymmetricHazardLayout", rejectsAsymmetricHazardLayout},
		{"rejectsOverlappingArenaHazards", rejectsOverlappingArenaHazards},
		{"rejectsInvalidArenaDimensions", rejectsInvalidArenaDimensions},
		{"rejectsOutOfBoundsArenaObjects", rejectsOutOfBoundsArenaObjects},
		{"rejectsMissingBases", rejectsMissingBases},
		{"rejectsInvalidObjectiveSpawn", rejectsInvalidObjectiveSpawn},
	};

	int failed = 0;
	for (const auto& [name, test] : tests)
	{
		try
		{
			test();
			std::cout << "[PASS] " << name << '\n';
		}
		catch (const std::exception& ex)
		{
			++failed;
			std::cerr << "[FAIL] " << name << ": " << ex.what() << '\n';
		}
	}

	if (failed != 0)
	{
		std::cerr << failed << " test(s) failed\n";
		return 1;
	}

	std::cout << tests.size() << " test(s) passed\n";
	return 0;
}
