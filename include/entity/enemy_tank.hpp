#pragma once

#include "entity/tank.hpp"
#include "entity/enemy_type.hpp"

class Map;

class EnemyTank : public Tank {
public:
    EnemyTank(float x, float y, EnemyType type);

    void update(float deltaTime, std::vector<Bullet>& bullets) override;
    void update(float deltaTime,std::vector<Bullet>& bullets,const Map& map,const std::vector<sf::FloatRect>& tankBlocks);
    void setMoveDirection(Direction dir);
    void stopMoving();
    void tryFire(std::vector<Bullet>& bullets);
    EnemyType getType() const { return type_; }

private:
    EnemyType type_;
    bool isMoving_;

    sf::Vector2f movementFor(float deltaTime) const;
};
