#pragma once

#include <SFML/Graphics.hpp>

class Base {
public:
    Base(float x, float y);

    void takeDamage(int damage);
    bool isAlive() const { return hp_ > 0; }
    int getHp() const noexcept { return hp_; }
    void setHp(int hp) noexcept;
    void draw(sf::RenderWindow& window) const;
    sf::FloatRect getBounds() const { return shape_.getGlobalBounds(); }

private:
    int hp_;
    sf::RectangleShape shape_;
};
