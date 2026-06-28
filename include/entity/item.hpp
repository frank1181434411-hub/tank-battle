#pragma once

#include <SFML/Graphics.hpp>

enum class ItemType {
    HealthPack,
    DamagePack
};

class Item {
public:
    Item(sf::Vector2f pos, ItemType type);

    void draw(sf::RenderWindow& window) const;
    bool isAlive() const { return isAlive_; }
    void pickUp() { isAlive_ = false; }
    ItemType getType() const { return type_; }
    sf::FloatRect getBounds() const { return shape_.getGlobalBounds(); }

private:
    ItemType type_;
    bool isAlive_;
    sf::RectangleShape shape_;
};
