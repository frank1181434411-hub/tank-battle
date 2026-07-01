#pragma once

#include <SFML/Graphics.hpp>

#include "entity/entity.hpp"

class Bullet {
public:
    Bullet(sf::Vector2f startPos, Direction dir, Faction faction, int damage, float speed, int ownerId);

    void update(float deltaTime);
    void draw(sf::RenderWindow& window) const;
    bool isAlive() const { return isAlive_; }
    void destroy() { isAlive_ = false; }
    int getDamage() const { return damage_; }
    int getOwnerId() const { return ownerId_; }
    Faction getFaction() const { return faction_; }
    sf::FloatRect getBounds() const { return shape_.getGlobalBounds(); }
    sf::Vector2f getPosition() const noexcept { return position_; }
    sf::Vector2f getVelocity() const noexcept { return velocity_; }

private:
    sf::Vector2f position_;
    sf::Vector2f velocity_;
    int damage_;
    int ownerId_;
    Faction faction_;
    bool isAlive_;
    sf::CircleShape shape_;
};
