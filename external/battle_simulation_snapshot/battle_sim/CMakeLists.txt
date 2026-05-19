cmake_minimum_required(VERSION 3.16)
project(battle_simulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BATTLE_SIM_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

set(BATTLE_SIM_SOURCES
    src/App/RegisterBuiltInFeaturePacks.cpp
    src/App/RuntimeAssembly.cpp
    src/App/ScenarioConfig.cpp
    src/App/ScenarioRunner.cpp
    src/App/TraceInspector.cpp
    src/Core/Ecs/World.cpp
    src/Core/Engine/EngineRunner.cpp
    src/Core/Engine/Mutation.cpp
    src/Core/Registry/CoreHandlers.cpp
    src/Features/Battle/BattleFeaturePack.cpp
    src/Features/Battle/EntityArchetypeRegistry.cpp
    src/Features/Battle/BattleSimulationFacade.cpp
    src/Features/Battle/Components/BattleComponentAccess.cpp
    src/Features/Battle/Conditions/BattleConditionHandlers.cpp
    src/Features/Battle/Effects/BattleEffects.cpp
    src/Features/Battle/Mutations/DamageEntityMutation.cpp
    src/Features/Battle/Mutations/HealEntityMutation.cpp
    src/Features/Battle/Mutations/MoveEntityOnBattleMapMutation.cpp
    src/Features/Battle/Mutations/RemoveBattleEntityMutation.cpp
    src/Features/Battle/Policies/BattlePolicyAccess.cpp
    src/Features/Battle/Resources/BattleMapResource.cpp
    src/Features/Battle/Selectors/BattleTargetSelectors.cpp
    src/Features/Battle/Selectors/SpatialQuery.cpp
    src/Features/Battle/Systems/BattleMarchSystem.cpp
    src/Features/Battle/Systems/BattleMovementSystem.cpp
    src/Features/Battle/Systems/BattleSpawnSystem.cpp
    src/Features/Battle/Systems/BattleTurnSystem.cpp
    src/Features/Battle/Values/BattleValueResolvers.cpp
    src/Features/Register.cpp
    src/Features/UnitsClassic/ClassicActionRuleRecipes.cpp
    src/Features/UnitsClassic/ClassicUnitArchetypes.cpp
    src/Features/UnitsClassic/DataDrivenArchetypes.cpp
    src/Features/UnitsClassic/UnitsClassicFeaturePack.cpp
    src/IO/LegacyCommands/CommandParser.cpp
    src/IO/LegacyCommands/LegacyEventAdapter.cpp
    src/IO/LegacyCommands/LegacyIoFeaturePack.cpp
    src/IO/LegacyCommands/RegisterLegacyCommands.cpp
    src/IO/Trace/JsonTraceWriter.cpp
)

add_library(battle_sim_lib ${BATTLE_SIM_SOURCES})
target_include_directories(battle_sim_lib PUBLIC src/)

if(MSVC)
    target_compile_definitions(battle_sim_lib PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(battle_sim_lib PRIVATE /W4 /permissive-)
    if(BATTLE_SIM_WARNINGS_AS_ERRORS)
        target_compile_options(battle_sim_lib PRIVATE /WX)
    endif()
else()
    target_compile_options(battle_sim_lib PRIVATE -Wall -Wextra -Wpedantic)
    if(BATTLE_SIM_WARNINGS_AS_ERRORS)
        target_compile_options(battle_sim_lib PRIVATE -Werror)
    endif()
endif()

add_executable(battle_sim src/main.cpp)
target_link_libraries(battle_sim PRIVATE battle_sim_lib)

include(CTest)

if(BUILD_TESTING)
add_executable(scenario_behavior_tests tests/ScenarioBehaviorTests.cpp)
target_link_libraries(scenario_behavior_tests PRIVATE battle_sim_lib)
add_test(NAME scenario_behavior_tests COMMAND scenario_behavior_tests)
set_tests_properties(scenario_behavior_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ecs_world_tests tests/EcsWorldTests.cpp)
target_link_libraries(ecs_world_tests PRIVATE battle_sim_lib)
add_test(NAME ecs_world_tests COMMAND ecs_world_tests)
set_tests_properties(ecs_world_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ecs_action_rule_tests tests/EcsActionRuleTests.cpp)
target_link_libraries(ecs_action_rule_tests PRIVATE battle_sim_lib)
add_test(NAME ecs_action_rule_tests COMMAND ecs_action_rule_tests)
set_tests_properties(ecs_action_rule_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ecs_action_executor_tests tests/EcsActionExecutorTests.cpp)
target_link_libraries(ecs_action_executor_tests PRIVATE battle_sim_lib)
add_test(NAME ecs_action_executor_tests COMMAND ecs_action_executor_tests)
set_tests_properties(ecs_action_executor_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(deterministic_scenario_tests tests/DeterministicScenarioTests.cpp)
target_link_libraries(deterministic_scenario_tests PRIVATE battle_sim_lib)
add_test(NAME deterministic_scenario_tests COMMAND deterministic_scenario_tests)
set_tests_properties(deterministic_scenario_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
