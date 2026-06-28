#pragma once

#include "entity/item.hpp"
#include "entity/tank.hpp"

class PlayerTank : public Tank {
public:
    PlayerTank(float x, float y);

    void update(float deltaTime, std::vector<Bullet>& bullets) override;
    void collectItem(Item& item);
    bool hasDamageBuff() const { return damageBuffTimer_ > 0.f; }

private:
    float damageBuffTimer_;

    void handleMovement(float deltaTime);
};
