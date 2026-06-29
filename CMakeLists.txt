cmake_minimum_required(VERSION 3.28)

project(TankBattle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(TANK_BATTLE_BUILD_TESTS "Build Tank Battle tests" OFF)
option(TANK_BATTLE_ENABLE_RL "Enable reinforcement learning features" OFF)
option(TANK_BATTLE_ENABLE_DEBUG_UI "Enable debug UI" OFF)

include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
)

FetchContent_MakeAvailable(SFML)

add_executable(TankBattle
    src/main.cpp
    src/core/game.cpp
    src/core/resource_manager.cpp
    src/entity/base.cpp
    src/entity/bullet.cpp
    src/entity/enemy_tank.cpp
    src/entity/item.cpp
    src/entity/player_tank.cpp
    src/entity/tank.cpp
    src/world/collision.cpp
    src/world/map.cpp
    src/world/tile.cpp
)

target_include_directories(TankBattle PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(TankBattle PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_definitions(TankBattle PRIVATE
    $<$<BOOL:${TANK_BATTLE_ENABLE_RL}>:TANK_BATTLE_ENABLE_RL>
    $<$<BOOL:${TANK_BATTLE_ENABLE_DEBUG_UI}>:TANK_BATTLE_ENABLE_DEBUG_UI>
)

target_link_libraries(TankBattle PRIVATE
    SFML::Graphics
    SFML::Audio
    SFML::Network
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET TankBattle POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                "$<TARGET_FILE_DIR:TankBattle>/assets"
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config")
    add_custom_command(TARGET TankBattle POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/config"
                "$<TARGET_FILE_DIR:TankBattle>/config"
    )
endif()
