#include"ai/enemy_ai.hpp"

#include<array>
#include<cstddef>
#include<random>

namespace
{
constexpr float PositionEpsilonSquared=0.01f;
constexpr std::array<Direction,4> AllDirections{Direction::Up,Direction::Down,Direction::Left,Direction::Right};
Direction randomDirection(std::mt19937& random)
{
    std::uniform_int_distribution<std::size_t> distribution(0,AllDirections.size()-1);
    return AllDirections[distribution(random)];
}
}