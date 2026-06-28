#include "entity/base.hpp"

Base::Base(float x, float y)
    : hp_(100),
      shape_()
{
    shape_.setSize({40.f, 40.f});
    shape_.setFillColor(sf::Color::Blue);
    shape_.setOrigin({20.f, 20.f});
    shape_.setPosition({x, y});
}

void Base::takeDamage(int damage) {
    hp_ -= damage;
    if (hp_ < 0) {
        hp_ = 0;
    }
}

void Base::draw(sf::RenderWindow& window) const {
    if (isAlive()) {
        window.draw(shape_);
    }
}
