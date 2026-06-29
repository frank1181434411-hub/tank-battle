#pragma once

#include "entity/item.hpp"
#include "entity/tank.hpp"

class Map;

class PlayerTank : public Tank {
public:
    PlayerTank(float x, float y);

    void update(float deltaTime, std::vector<Bullet>& bullets) override;
    void update(float deltaTime,std::vector<Bullet>& bullets,const Map& map,const std::vector<sf::FloatRect>& tankBlocks);
    void collectItem(Item& item);
    bool hasDamageBuff() const { return damageBuffTimer_ > 0.f; }

private:
    float damageBuffTimer_;

    void handleMovement(float deltaTime);
    void handleMovement(float deltaTime,const Map& map,const std::vector<sf::FloatRect>& tankBlocks);
};
