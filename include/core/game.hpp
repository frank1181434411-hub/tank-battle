#pragma once

#include <SFML/Graphics.hpp>

#include <optional>
#include <vector>

#include "entity/base.hpp"
#include "entity/bullet.hpp"
#include "entity/enemy_tank.hpp"
#include "entity/item.hpp"
#include "entity/player_tank.hpp"
#include "world/map.hpp"

enum class GameStatus {
    Running,
    Victory,
    Defeat
};

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();
    void updateStatus();

    sf::RenderWindow window_;
    Map map_;
    std::optional<PlayerTank> player_;
    std::optional<Base> base_;
    std::vector<EnemyTank> enemies_;
    std::vector<Bullet> bullets_;
    std::vector<Item> items_;
    GameStatus status_;
};
