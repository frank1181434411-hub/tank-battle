#include "core/game.hpp"
#include "world/collision.hpp"

#include <cstddef>
#include <optional>

Game::Game()
    : window_(sf::VideoMode({static_cast<unsigned int>(prototype::WindowWidth),
                             static_cast<unsigned int>(prototype::WindowHeight)}),
              "Retro Tank Battle"),
      map_(),
      player_(),
      base_(),
      enemies_(),
      bullets_(),
      items_(),
      status_(GameStatus::Running)
{
    window_.setFramerateLimit(60);

    const bool mapLoaded=map_.loadFromFile("assets/maps/map_test.txt");

    sf::Vector2f playerPosition(400.f,500.f);
    if(mapLoaded && map_.hasPlayerSpawn()) playerPosition=map_.playerSpawn();
    player_.emplace(playerPosition.x,playerPosition.y);

    sf::Vector2f basePosition(400.f,550.f);
    if(mapLoaded && map_.hasBasePosition()) basePosition=map_.basePosition();
    base_.emplace(basePosition.x,basePosition.y);

    items_.emplace_back(sf::Vector2f{300.f, 300.f}, ItemType::HealthPack);
    items_.emplace_back(sf::Vector2f{400.f, 300.f}, ItemType::DamagePack);

    if(mapLoaded && !map_.enemySpawns().empty())
    {
        const auto& enemySpawns=map_.enemySpawns();
        for(std::size_t i=0;i<enemySpawns.size();++i)
        {
            const EnemyType type=i%2==0?EnemyType::Light:EnemyType::Heavy;
            enemies_.emplace_back(enemySpawns[i].x,enemySpawns[i].y,type);
        }
    }
    else
    {
        enemies_.emplace_back(100.f,100.f,EnemyType::Light);
        enemies_.emplace_back(700.f,100.f,EnemyType::Heavy);
    }

    if(!enemies_.empty()) enemies_[0].setMoveDirection(Direction::Down);
    if(enemies_.size()>1) enemies_[1].setMoveDirection(Direction::Left);
}

void Game::run() {
    sf::Clock clock;
    while (window_.isOpen()) {
        const float deltaTime = clock.restart().asSeconds();
        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::processEvents() {
    while (const std::optional event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window_.close();
        }
    }
}

void Game::update(float deltaTime) {
    if (status_ != GameStatus::Running || !player_ || !base_) {
        return;
    }

    std::vector<sf::FloatRect> playerTankBlocks;
    if(base_->isAlive())
        playerTankBlocks.push_back(base_->getBounds());
    for(const auto& enemy:enemies_)
    {
        if(enemy.isAlive())
            playerTankBlocks.push_back(enemy.getBounds());
    }
    player_->update(deltaTime,bullets_,map_,playerTankBlocks);

    for (auto& enemy : enemies_) {
        std::vector<sf::FloatRect> enemyTankBlocks;
        if(player_->isAlive())
            enemyTankBlocks.push_back(player_->getBounds());
        if(base_->isAlive())
            enemyTankBlocks.push_back(base_->getBounds());
        for(const auto& other:enemies_)
        {
            if(other.getId()!=enemy.getId() && other.isAlive())
                enemyTankBlocks.push_back(other.getBounds());
        }
        enemy.update(deltaTime,bullets_,map_,enemyTankBlocks);
        enemy.tryFire(bullets_);
    }

    for (auto& item : items_) {
        if (item.isAlive() && player_->getBounds().findIntersection(item.getBounds()).has_value()) {
            player_->collectItem(item);
        }
    }

    for (auto bullet = bullets_.begin(); bullet != bullets_.end();) {
        bullet->update(deltaTime);
        bool hit = false;

        int hitTileX=0;
        int hitTileY=0;
        if(Collision::findBulletHitTile(map_,bullet->getBounds(),hitTileX,hitTileY))
        {
            const Tile* tile=map_.tileAt(hitTileX,hitTileY);
            if(tile!=nullptr && tile->type()==TileType::Brick)
                map_.setTile(hitTileX,hitTileY,TileType::Empty);
            hit=true;
        }
        else if(Collision::hitsMapBounds(map_,bullet->getBounds()))
            hit=true;

        if (!hit && bullet->getFaction() == Faction::Player) {
            for (auto& enemy : enemies_) {
                if (enemy.isAlive() && bullet->getBounds().findIntersection(enemy.getBounds()).has_value()) {
                    enemy.takeDamage(bullet->getDamage());
                    hit = true;
                    break;
                }
            }
        } else if (!hit && bullet->getFaction() == Faction::Enemy) {
            if (player_->isAlive() && bullet->getBounds().findIntersection(player_->getBounds()).has_value()) {
                player_->takeDamage(bullet->getDamage());
                hit = true;
            } else if (base_->isAlive() && bullet->getBounds().findIntersection(base_->getBounds()).has_value()) {
                base_->takeDamage(bullet->getDamage());
                hit = true;
            }
        }

        if (hit || !bullet->isAlive()) {
            bullet = bullets_.erase(bullet);
        } else {
            ++bullet;
        }
    }

    for (auto enemy = enemies_.begin(); enemy != enemies_.end();) {
        if (!enemy->isAlive()) {
            enemy = enemies_.erase(enemy);
        } else {
            ++enemy;
        }
    }

    for (auto item = items_.begin(); item != items_.end();) {
        if (!item->isAlive()) {
            item = items_.erase(item);
        } else {
            ++item;
        }
    }

    updateStatus();
}

void Game::render() {
    window_.clear(sf::Color::Black);

    map_.drawLayer(window_,0);
    if(base_) base_->draw(window_);
    for (const auto& item : items_) {
        item.draw(window_);
    }

    if(player_) player_->draw(window_);
    for (const auto& enemy : enemies_) {
        enemy.draw(window_);
    }

    for (const auto& bullet : bullets_) {
        bullet.draw(window_);
    }

    map_.drawLayer(window_,1);

    window_.display();
}

void Game::updateStatus() {
    if(!player_ || !base_) return;

    if (!base_->isAlive() || !player_->isAlive()) {
        status_ = GameStatus::Defeat;
        window_.setTitle("Retro Tank Battle - Defeat");
    } else if (enemies_.empty()) {
        status_ = GameStatus::Victory;
        window_.setTitle("Retro Tank Battle - Victory");
    }
}
