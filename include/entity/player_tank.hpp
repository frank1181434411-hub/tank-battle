#pragma once

#include "entity/item.hpp"
#include "entity/tank.hpp"

class Map;

class PlayerTank : public Tank {
public:
    PlayerTank(float x, float y);

    void update(float deltaTime, std::vector<Bullet>& bullets) override;
    void update(float deltaTime,std::vector<Bullet>& bullets,const Map& map,const std::vector<sf::FloatRect>& tankBlocks);
    void updateWithInput(float deltaTime,std::vector<Bullet>& bullets,const Map& map,const std::vector<sf::FloatRect>& tankBlocks,bool moveUp,bool moveDown,bool moveLeft,bool moveRight,bool firePressed);
    void collectItem(Item& item);
    bool hasDamageBuff() const { return damageBuffTimer_ > 0.f; }
    void setColor(sf::Color color);

private:
    float damageBuffTimer_;

    void handleMovement(float deltaTime);
    void handleMovement(float deltaTime,const Map& map,const std::vector<sf::FloatRect>& tankBlocks);
    void handleMovementWithInput(float deltaTime,const Map& map,const std::vector<sf::FloatRect>& tankBlocks,bool moveUp,bool moveDown,bool moveLeft,bool moveRight);
};
