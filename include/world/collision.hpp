#pragma once
#include <SFML/Graphics.hpp>
#include "world/map.hpp"

namespace Collision {
bool hitsTankBlock(const Map& map,const sf::FloatRect& bounds) noexcept;
bool hitsBulletBlock(const Map& map,const sf::FloatRect& bounds) noexcept;
bool hitsMapBounds(const Map& map,const sf::FloatRect& bounds) noexcept;
bool findBulletHitTile(const Map& map,const sf::FloatRect& bounds,int& tileX,int& tileY) noexcept;
}
