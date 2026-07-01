#pragma once

#include"entity/entity.hpp"

struct TankCommand
{
    Direction direction=Direction::Down;
    bool move=false,fire=false;
};