#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ai/ai_controller.hpp"
#include "entity/base.hpp"
#include "entity/bullet.hpp"
#include "entity/enemy_tank.hpp"
#include "entity/item.hpp"
#include "entity/player_tank.hpp"
#include "network/network_session.hpp"
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
    void startHost(unsigned short port);
    void joinHost(const std::string& address,unsigned short port);
    void leaveNetworkGame();

private:
    void processEvents();
    void update(float deltaTime);
    void updateOffline(float deltaTime);
    void updateHost(float deltaTime);
    void updateClient(float deltaTime);
    void render();
    void updateStatus();
    void collectLocalInput(PlayerInputState& input);
    void applySnapshot(const GameSnapshot& snapshot);
    GameSnapshot createSnapshot() const;
    void addEnemy(float x,float y,EnemyType type);
    void removeDeadEnemies();

    sf::RenderWindow window_;
    Map map_;
    NetworkSession networkSession_;
    std::optional<PlayerTank> playerOne_;
    std::optional<PlayerTank> playerTwo_;
    std::optional<Base> base_;
    std::vector<EnemyTank> enemies_;
    std::vector<std::unique_ptr<AIController>> enemyAis_;
    std::vector<Bullet> bullets_;
    std::vector<Item> items_;
    PlayerInputState localInput_;
    PlayerInputState remoteInput_;
    bool hasTwoPlayers_=false;
    GameStatus status_;
};
