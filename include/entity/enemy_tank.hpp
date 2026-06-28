#pragma once

#include "entity/tank.hpp"

enum class EnemyType {
    Light,
    Heavy
};

class EnemyTank : public Tank {
public:
    EnemyTank(float x, float y, EnemyType type);

    void update(float deltaTime, std::vector<Bullet>& bullets) override;
    void setMoveDirection(Direction dir);
    void stopMoving();
    void tryFire(std::vector<Bullet>& bullets);
    EnemyType getType() const { return type_; }

private:
    EnemyType type_;
    bool isMoving_;
};
