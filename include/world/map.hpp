#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "world/tile.hpp"

class Map {
public:
    Map()=default;

    bool loadFromFile(const std::string& filePath);
    bool loadFromLines(const std::vector<std::string>& lines);
    void clear();
    void resize(int width,int height,TileType fillType=TileType::Empty);

    bool empty() const noexcept;
    int width() const noexcept;
    int height() const noexcept;
    bool isInBounds(int x,int y) const noexcept;

    Tile* tileAt(int x,int y) noexcept;
    const Tile* tileAt(int x,int y) const noexcept;
    void setTile(int x,int y,TileType type);

    sf::Vector2f tilePosition(int x,int y) const noexcept;
    sf::Vector2f tileCenter(int x,int y) const noexcept;

    bool hasPlayerSpawn() const noexcept;
    sf::Vector2f playerSpawn() const noexcept;

    bool hasBasePosition() const noexcept;
    sf::Vector2f basePosition() const noexcept;

    const std::vector<sf::Vector2f>& enemySpawns() const noexcept;
    const std::string& lastError() const noexcept;

    void draw(sf::RenderWindow& window) const;

private:
    int index(int x,int y) const noexcept;
    void readSpecialPoint(char value,int x,int y);

    int width_=0;
    int height_=0;
    std::vector<Tile> tiles_;
    bool hasPlayerSpawn_=false;
    bool hasBasePosition_=false;
    sf::Vector2f playerSpawn_{0.f,0.f};
    sf::Vector2f basePosition_{0.f,0.f};
    std::vector<sf::Vector2f> enemySpawns_;
    std::string lastError_;
};
