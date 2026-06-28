#include "entity/item.hpp"

Item::Item(sf::Vector2f pos, ItemType type)
    : type_(type),
      isAlive_(true),
      shape_()
{
    shape_.setSize({20.f, 20.f});
    shape_.setOrigin({10.f, 10.f});
    shape_.setPosition(pos);

    if (type == ItemType::HealthPack) {
        shape_.setFillColor(sf::Color::Red);
    } else {
        shape_.setFillColor(sf::Color::Magenta);
    }
}

void Item::draw(sf::RenderWindow& window) const {
    if (isAlive_) {
        window.draw(shape_);
    }
}
