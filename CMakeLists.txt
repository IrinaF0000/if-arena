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
    src/battle_core/placeholder.cpp
)
target_include_directories(battle_core PUBLIC src/battle_core)

add_library(battle_protocol STATIC
    src/battle_protocol/placeholder.cpp
)
target_include_directories(battle_protocol PUBLIC src/battle_protocol)

add_library(battle_backend STATIC
    src/battle_backend/placeholder.cpp
)
target_include_directories(battle_backend PUBLIC src/battle_backend)
target_link_libraries(battle_backend PUBLIC battle_core battle_protocol)

if(BATTLE_BUILD_TCP_TRANSPORT)
    add_library(battle_transport_tcp STATIC
        src/battle_transport_tcp/placeholder.cpp
    )
    target_include_directories(battle_transport_tcp PUBLIC src/battle_transport_tcp)
    target_link_libraries(battle_transport_tcp PUBLIC battle_protocol battle_backend)
endif()

if(BATTLE_BUILD_WS_TRANSPORT)
    add_library(battle_transport_ws STATIC
        src/battle_transport_ws/placeholder.cpp
    )
    target_include_directories(battle_transport_ws PUBLIC src/battle_transport_ws)
    target_link_libraries(battle_transport_ws PUBLIC battle_protocol battle_backend)
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
    add_executable(battle_cli_client src/battle_cli_client/placeholder.cpp)
    target_link_libraries(battle_cli_client PRIVATE battle_protocol)
endif()

if(BATTLE_BUILD_LOAD_CLIENT)
    add_executable(battle_load_client tools/load_client/placeholder.cpp)
    target_link_libraries(battle_load_client PRIVATE battle_protocol)
endif()

if(BATTLE_BUILD_QT_CLIENT)
    # Placeholder target only. Replace with a real Qt Widgets target after Qt dependency setup.
    add_executable(battle_qt_client src/battle_qt_client/placeholder.cpp)
    target_link_libraries(battle_qt_client PRIVATE battle_protocol)
endif()

if(BATTLE_BUILD_TESTS)
    enable_testing()
endif()
