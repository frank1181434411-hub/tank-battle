#include "world/tile.hpp"

Tile::Tile(TileType type) noexcept : type_(type)
{
}

TileType Tile::type() const noexcept{return type_;}

void Tile::setType(TileType type) noexcept{type_=type;}

bool Tile::blocksTank() const noexcept
{
    return (type_==TileType::Brick || type_==TileType::Steel || type_==TileType::Water);
}

bool Tile::blocksBullet() const noexcept
{
    return (type_==TileType::Brick || type_==TileType::Steel);
}

bool Tile::isDestructible() const noexcept{return type_==TileType::Brick;}

bool Tile::isDrawable() const noexcept{return type_!=TileType::Empty;}

int Tile::drawLayer() const noexcept{return type_==TileType::Grass?2:1;}

char Tile::toChar() const noexcept
{
    if(type_==TileType::Empty) return '.';
    else if(type_==TileType::Brick) return '#';
    else if(type_==TileType::Steel) return 'S';
    else if(type_==TileType::Water) return 'W';
    else if(type_==TileType::Grass) return 'G';
    else return '.';
}

TileType Tile::fromChar(char value) noexcept
{
    if(value=='.' || value==' ' || value=='P' || value=='E' || value=='B') return TileType::Empty;
    else if(value=='#') return TileType::Brick;
    else if(value=='S') return TileType::Steel;
    else if(value=='W') return TileType::Water;
    else if(value=='G') return TileType::Grass;
    else return TileType::Empty;
}

sf::Color Tile::color() const
{
    if(type_==TileType::Empty) return sf::Color::Transparent;
    else if(type_==TileType::Brick) return sf::Color(156,74,43);
    else if(type_==TileType::Steel) return sf::Color(130,136,144);
    else if(type_==TileType::Water) return sf::Color(42,103,188);
    else if(type_==TileType::Grass) return sf::Color(54,136,70);
    else return sf::Color::Transparent;
}
