#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

#include "entity/bullet.hpp"

class Tank {
public:
    Tank(float x, float y, float speed, int maxHp, Faction faction, float fireCooldown, float bulletSpeed);
    virtual ~Tank() = default;

    virtual void update(float deltaTime, std::vector<Bullet>& bullets) = 0;
    void draw(sf::RenderWindow& window) const;
    void takeDamage(int damage);
    bool heal(int amount);

    bool isAlive() const { return hp_ > 0; }
    int getHp() const { return hp_; }
    int getMaxHp() const { return maxHp_; }
    int getId() const { return id_; }
    sf::FloatRect getBounds() const { return bodyShape_.getGlobalBounds(); }

protected:
    sf::Vector2f position_;
    Direction dir_;
    float speed_;
    int hp_;
    int maxHp_;
    Faction faction_;
    int currentBulletDamage_;
    float fireCooldown_;
    float cooldownTimer_;
    float bulletSpeed_;
    int id_;
    sf::RectangleShape bodyShape_;
    sf::RectangleShape turretShape_;

    void fire(std::vector<Bullet>& bullets);
    void clampToWindow();
    void updateShapePositions();
    void updateTurretRotation();
};
