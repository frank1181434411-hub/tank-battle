#include "world/collision.hpp"
#include <algorithm>
#include <cmath>

namespace {
bool overlapsBlockedTile(const Map& map,const sf::FloatRect& bounds,bool checkBulletBlock) noexcept
{
    if(map.empty()) return false;
    if(Collision::hitsMapBounds(map,bounds)) return true;

    const float left=bounds.position.x;
    const float top=bounds.position.y;
    const float right=bounds.position.x+bounds.size.x;
    const float bottom=bounds.position.y+bounds.size.y;

    const int startX=std::max(0,static_cast<int>(std::floor(left/Tile::DefaultSize)));
    const int startY=std::max(0,static_cast<int>(std::floor(top/Tile::DefaultSize)));
    const int endX=std::min(map.width()-1,static_cast<int>(std::floor((right-0.001f)/Tile::DefaultSize)));
    const int endY=std::min(map.height()-1,static_cast<int>(std::floor((bottom-0.001f)/Tile::DefaultSize)));

    for(int y=startY;y<=endY;++y)
    {
        for(int x=startX;x<=endX;++x)
        {
            const Tile* tile=map.tileAt(x,y);
            if(tile==nullptr) return true;
            if(checkBulletBlock && tile->blocksBullet()) return true;
            if(!checkBulletBlock && tile->blocksTank()) return true;
        }
    }

    return false;
}
}

bool Collision::hitsTankBlock(const Map& map,const sf::FloatRect& bounds) noexcept
{
    return overlapsBlockedTile(map,bounds,false);
}

bool Collision::hitsBulletBlock(const Map& map,const sf::FloatRect& bounds) noexcept
{
    return overlapsBlockedTile(map,bounds,true);
}

bool Collision::findBulletHitTile(const Map& map,const sf::FloatRect& bounds,int& tileX,int& tileY) noexcept
{
    if(map.empty()) return false;
    if(hitsMapBounds(map,bounds)) return false;

    const float left=bounds.position.x;
    const float top=bounds.position.y;
    const float right=bounds.position.x+bounds.size.x;
    const float bottom=bounds.position.y+bounds.size.y;

    const int startX=std::max(0,static_cast<int>(std::floor(left/Tile::DefaultSize)));
    const int startY=std::max(0,static_cast<int>(std::floor(top/Tile::DefaultSize)));
    const int endX=std::min(map.width()-1,static_cast<int>(std::floor((right-0.001f)/Tile::DefaultSize)));
    const int endY=std::min(map.height()-1,static_cast<int>(std::floor((bottom-0.001f)/Tile::DefaultSize)));

    for(int y=startY;y<=endY;++y)
    {
        for(int x=startX;x<=endX;++x)
        {
            const Tile* tile=map.tileAt(x,y);
            if(tile!=nullptr && tile->blocksBullet())
            {
                tileX=x;
                tileY=y;
                return true;
            }
        }
    }

    return false;
}

bool Collision::hitsMapBounds(const Map& map,const sf::FloatRect& bounds) noexcept
{
    if(map.empty()) return false;

    const float rightLimit=static_cast<float>(map.width())*Tile::DefaultSize;
    const float bottomLimit=static_cast<float>(map.height())*Tile::DefaultSize;

    return (bounds.position.x<0.f ||
            bounds.position.y<0.f ||
            bounds.position.x+bounds.size.x>rightLimit ||
            bounds.position.y+bounds.size.y>bottomLimit);
}
