cmake_minimum_required(VERSION 3.24)
project(if_arena LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BATTLE_BUILD_SERVER_APP "Build combined backend server app" ON)
option(BATTLE_BUILD_TCP_TRANSPORT "Build raw TCP transport" ON)
option(BATTLE_BUILD_WS_TRANSPORT "Build WebSocket transport" ON)
option(BATTLE_BUILD_QT_CLIENT "Build Qt desktop client" OFF)
option(BATTLE_BUILD_CLI_CLIENT "Build CLI client" ON)
option(BATTLE_BUILD_LOAD_CLIENT "Build load client" ON)
option(BATTLE_BUILD_TESTS "Build tests" ON)
option(BATTLE_ENABLE_COVERAGE "Build with GNU/Clang coverage instrumentation" OFF)

if(BATTLE_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-O0 -g --coverage)
        add_link_options(--coverage)
    else()
        message(FATAL_ERROR "BATTLE_ENABLE_COVERAGE requires GNU or Clang")
    endif()
endif()

add_library(battle_core STATIC
    src/battle_core/BattleEngine.cpp
)
target_include_directories(battle_core PUBLIC src/battle_core)

add_library(battle_protocol STATIC
    src/battle_protocol/Protocol.cpp
)
target_include_directories(battle_protocol PUBLIC src/battle_protocol)

add_library(battle_backend_api INTERFACE)
target_include_directories(battle_backend_api INTERFACE src/battle_backend)

add_library(battle_backend STATIC
    src/battle_backend/BackendFoundation.cpp
    src/battle_backend/ScenarioConfig.cpp
)
target_link_libraries(battle_backend PUBLIC battle_backend_api battle_core battle_protocol)

if(BATTLE_BUILD_TCP_TRANSPORT)
    add_library(battle_transport_tcp STATIC
        src/battle_transport_tcp/TcpFrameCodec.cpp
    )
    target_include_directories(battle_transport_tcp PUBLIC src/battle_transport_tcp)
    target_link_libraries(battle_transport_tcp PUBLIC battle_protocol battle_backend_api)
    if(WIN32)
        target_link_libraries(battle_transport_tcp PRIVATE ws2_32)
    endif()
endif()

if(BATTLE_BUILD_WS_TRANSPORT)
    add_library(battle_transport_ws STATIC
        src/battle_transport_ws/WebSocketSession.cpp
    )
    target_include_directories(battle_transport_ws PUBLIC src/battle_transport_ws)
    target_link_libraries(battle_transport_ws PUBLIC battle_protocol battle_backend_api)
    if(WIN32)
        target_link_libraries(battle_transport_ws PRIVATE ws2_32)
    endif()
endif()

if(BATTLE_BUILD_SERVER_APP)
    add_executable(battle_server_app src/battle_server_app/main.cpp)
    target_link_libraries(battle_server_app PRIVATE battle_backend)
    if(BATTLE_BUILD_TCP_TRANSPORT)
        target_link_libraries(battle_server_app PRIVATE battle_transport_tcp)
    endif()
    if(BATTLE_BUILD_WS_TRANSPORT)
        target_link_libraries(battle_server_app PRIVATE battle_transport_ws)
    endif()
endif()

if(BATTLE_BUILD_CLI_CLIENT)
    add_executable(battle_cli_client src/battle_cli_client/CliClientMain.cpp)
    target_link_libraries(battle_cli_client PRIVATE battle_protocol)
    if(WIN32)
        target_link_libraries(battle_cli_client PRIVATE ws2_32)
    endif()
endif()

if(BATTLE_BUILD_LOAD_CLIENT)
    add_executable(battle_load_client tools/load_client/LoadClientMain.cpp)
    target_link_libraries(battle_load_client PRIVATE battle_protocol)
endif()

if(BATTLE_BUILD_QT_CLIENT)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Svg)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Svg)
    add_executable(battle_qt_client
        src/battle_qt_client/assets.qrc
        src/battle_qt_client/QtClientMain.cpp
        src/battle_qt_client/game/ClientTypes.cpp
        src/battle_qt_client/game/ClientTypes.hpp
        src/battle_qt_client/game/CoordinateTransform.cpp
        src/battle_qt_client/game/CoordinateTransform.hpp
        src/battle_qt_client/game/MovementInputController.cpp
        src/battle_qt_client/game/MovementInputController.hpp
        src/battle_qt_client/game/ProtocolJson.cpp
        src/battle_qt_client/game/ProtocolJson.hpp
        src/battle_qt_client/network/NetworkClient.cpp
        src/battle_qt_client/network/NetworkClient.hpp
        src/battle_qt_client/ui/ArenaView.cpp
        src/battle_qt_client/ui/ArenaView.hpp
        src/battle_qt_client/ui/MainWindow.cpp
        src/battle_qt_client/ui/MainWindow.hpp
    )
    set_target_properties(battle_qt_client PROPERTIES AUTOMOC ON AUTORCC ON)
    target_include_directories(battle_qt_client PRIVATE src/battle_qt_client)
    target_link_libraries(battle_qt_client PRIVATE battle_protocol Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Svg)
endif()

if(BATTLE_BUILD_TESTS)
    enable_testing()

    add_executable(battle_core_tests tests/unit/core/BattleEngineFacadeTests.cpp)
    target_link_libraries(battle_core_tests PRIVATE battle_core)
    add_test(NAME battle_core_tests COMMAND battle_core_tests)

    add_executable(battle_protocol_tests tests/unit/protocol/ProtocolEnvelopeTests.cpp)
    target_link_libraries(battle_protocol_tests PRIVATE battle_protocol)
    add_test(NAME battle_protocol_tests COMMAND battle_protocol_tests)

    add_executable(battle_backend_tests tests/unit/backend/SessionRegistryTests.cpp)
    target_link_libraries(battle_backend_tests PRIVATE battle_backend)
    add_test(NAME battle_backend_tests COMMAND battle_backend_tests)

    if(BATTLE_BUILD_TCP_TRANSPORT)
        add_executable(battle_transport_tcp_tests tests/unit/transport/TcpFrameCodecTests.cpp)
        target_link_libraries(battle_transport_tcp_tests PRIVATE battle_transport_tcp)
        add_test(NAME battle_transport_tcp_tests COMMAND battle_transport_tcp_tests)
    endif()

    if(BATTLE_BUILD_WS_TRANSPORT)
        add_executable(battle_transport_ws_tests tests/unit/transport/WebSocketSessionTests.cpp)
        target_link_libraries(battle_transport_ws_tests PRIVATE battle_transport_ws)
        add_test(NAME battle_transport_ws_tests COMMAND battle_transport_ws_tests)
    endif()

    add_executable(battle_security_tests tests/security/TelegramAuthTests.cpp)
    target_link_libraries(battle_security_tests PRIVATE battle_backend)
    add_test(NAME battle_security_tests COMMAND battle_security_tests)

    add_executable(battle_qt_client_transform_tests
        src/battle_qt_client/game/CoordinateTransform.cpp
        src/battle_qt_client/tests/CoordinateTransformTests.cpp
    )
    target_include_directories(battle_qt_client_transform_tests PRIVATE src/battle_qt_client)
    add_test(NAME battle_qt_client_transform_tests COMMAND battle_qt_client_transform_tests)

    add_executable(battle_qt_client_movement_input_tests
        src/battle_qt_client/game/MovementInputController.cpp
        src/battle_qt_client/tests/MovementInputControllerTests.cpp
    )
    target_include_directories(battle_qt_client_movement_input_tests PRIVATE src/battle_qt_client)
    add_test(NAME battle_qt_client_movement_input_tests COMMAND battle_qt_client_movement_input_tests)

    if(BATTLE_BUILD_QT_CLIENT)
        add_executable(battle_qt_client_asset_tests
            src/battle_qt_client/assets.qrc
            src/battle_qt_client/tests/AssetResourceTests.cpp
        )
        set_target_properties(battle_qt_client_asset_tests PROPERTIES AUTOMOC ON AUTORCC ON)
        target_link_libraries(battle_qt_client_asset_tests PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Svg)
        add_test(NAME battle_qt_client_asset_tests COMMAND battle_qt_client_asset_tests)
        set_tests_properties(battle_qt_client_asset_tests PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin"
        )

        add_executable(battle_qt_client_arena_view_tests
            src/battle_qt_client/assets.qrc
            src/battle_qt_client/game/ClientTypes.cpp
            src/battle_qt_client/game/CoordinateTransform.cpp
            src/battle_qt_client/tests/ArenaViewOverlayTests.cpp
            src/battle_qt_client/ui/ArenaView.cpp
            src/battle_qt_client/ui/ArenaView.hpp
        )
        set_target_properties(battle_qt_client_arena_view_tests PROPERTIES AUTOMOC ON AUTORCC ON)
        target_include_directories(battle_qt_client_arena_view_tests PRIVATE src/battle_qt_client)
        target_link_libraries(battle_qt_client_arena_view_tests PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg)
        add_test(NAME battle_qt_client_arena_view_tests COMMAND battle_qt_client_arena_view_tests)
        set_tests_properties(battle_qt_client_arena_view_tests PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin"
        )
    endif()
endif()

if(BATTLE_ENABLE_COVERAGE)
    find_program(GCOVR_EXECUTABLE gcovr)
    if(GCOVR_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${GCOVR_EXECUTABLE}
                --root ${CMAKE_SOURCE_DIR}
                --filter ${CMAKE_SOURCE_DIR}/src
                --exclude ${CMAKE_SOURCE_DIR}/tests
                --html-details ${CMAKE_BINARY_DIR}/coverage/index.html
                --xml ${CMAKE_BINARY_DIR}/coverage/coverage.xml
                --print-summary
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Run tests and generate gcovr coverage report"
        )
    else()
        message(STATUS "gcovr not found; coverage instrumentation is enabled but the coverage target is unavailable")
    endif()
endif()
