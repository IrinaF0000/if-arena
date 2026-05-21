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
    # Placeholder target only. Replace with a real Qt Widgets target after Qt dependency setup.
    add_executable(battle_qt_client src/battle_qt_client/QtClientMain.cpp)
    target_link_libraries(battle_qt_client PRIVATE battle_protocol)
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
endif()
