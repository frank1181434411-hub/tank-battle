#pragma once
#include <SFML/Graphics.hpp>

enum class TileType {
    Empty,
    Brick,
    Steel,
    Water,
    Grass
};

class Tile {
public:
    static constexpr float DefaultSize=40.f;

    constexpr Tile()=default;
    explicit Tile(TileType type) noexcept;

    TileType type() const noexcept;
    void setType(TileType type) noexcept;

    bool blocksTank() const noexcept;
    bool blocksBullet() const noexcept;
    bool isDestructible() const noexcept;
    bool isDrawable() const noexcept;
    int drawLayer() const noexcept;
    char toChar() const noexcept;
    static TileType fromChar(char value) noexcept;
    sf::Color color() const;

private:
    TileType type_=TileType::Empty;
};
