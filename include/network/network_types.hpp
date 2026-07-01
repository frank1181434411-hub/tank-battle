#pragma once
#include <SFML/System.hpp>
#include <utility>
#include <vector>

enum class NetworkMode
{
    Offline,
    Host,
    Client
};

enum class NetworkState
{
    Disconnected,
    Connecting,
    Connected,
    Hosting,
    Failed
};

struct PlayerInputState
{
    bool moveUp = false;
    bool moveDown = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool fire = false;
};

struct TankSnapshot
{
    int id = 0;
    float x = 0.f;
    float y = 0.f;
    int direction = 0;
    int hp = 0;
    bool alive = false;
};

struct BulletSnapshot
{
    float x = 0.f;
    float y = 0.f;
    int faction = 0;
    int damage = 1;
    int ownerId = 0;
    bool alive = false;
    float dirX = 0.f;
    float dirY = 0.f;
};

struct ItemSnapshot
{
    float x = 0.f;
    float y = 0.f;
    int type = 0;
    bool alive = false;
};

struct GameSnapshot
{
    int status = 0;
    int baseHp = 0;
    bool baseAlive = false;

    TankSnapshot playerOne;
    TankSnapshot playerTwo;

    std::vector<TankSnapshot> enemies;
    std::vector<BulletSnapshot> bullets;
    std::vector<ItemSnapshot> items;
    std::vector<std::pair<int, int>> destroyedBricks;
};