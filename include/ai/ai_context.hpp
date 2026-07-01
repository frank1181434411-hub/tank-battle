#pragma once

#include<vector>

#include"entity/base.hpp"
#include"entity/bullet.hpp"
#include"entity/enemy_tank.hpp"
#include"entity/player_tank.hpp"
#include"world/map.hpp"

struct AIContext
{
    const Map& map;
    const std::vector<PlayerTank>& players;
    const Base& base;
    const std::vector<EnemyTank>& enemies;
    const std::vector<Bullet>& bullets;
};
