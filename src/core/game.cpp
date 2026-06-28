#include "core/game.hpp"

#include <optional>

Game::Game()
    : window_(sf::VideoMode({static_cast<unsigned int>(prototype::WindowWidth),
                             static_cast<unsigned int>(prototype::WindowHeight)}),
              "Retro Tank Battle"),
      player_(400.f, 500.f),
      base_(400.f, 550.f),
      enemies_(),
      bullets_(),
      items_(),
      status_(GameStatus::Running)
{
    window_.setFramerateLimit(60);

    items_.emplace_back(sf::Vector2f{300.f, 300.f}, ItemType::HealthPack);
    items_.emplace_back(sf::Vector2f{400.f, 300.f}, ItemType::DamagePack);

    enemies_.emplace_back(100.f, 100.f, EnemyType::Light);
    enemies_.emplace_back(700.f, 100.f, EnemyType::Heavy);
    enemies_[0].setMoveDirection(Direction::Down);
    enemies_[1].setMoveDirection(Direction::Left);
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
    if (status_ != GameStatus::Running) {
        return;
    }

    player_.update(deltaTime, bullets_);
    for (auto& enemy : enemies_) {
        enemy.update(deltaTime, bullets_);
        enemy.tryFire(bullets_);
    }

    for (auto& item : items_) {
        if (item.isAlive() && player_.getBounds().findIntersection(item.getBounds()).has_value()) {
            player_.collectItem(item);
        }
    }

    for (auto bullet = bullets_.begin(); bullet != bullets_.end();) {
        bullet->update(deltaTime);
        bool hit = false;

        if (bullet->getFaction() == Faction::Player) {
            for (auto& enemy : enemies_) {
                if (enemy.isAlive() && bullet->getBounds().findIntersection(enemy.getBounds()).has_value()) {
                    enemy.takeDamage(bullet->getDamage());
                    hit = true;
                    break;
                }
            }
        } else if (bullet->getFaction() == Faction::Enemy) {
            if (player_.isAlive() && bullet->getBounds().findIntersection(player_.getBounds()).has_value()) {
                player_.takeDamage(bullet->getDamage());
                hit = true;
            } else if (base_.isAlive() && bullet->getBounds().findIntersection(base_.getBounds()).has_value()) {
                base_.takeDamage(bullet->getDamage());
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

    base_.draw(window_);
    for (const auto& item : items_) {
        item.draw(window_);
    }

    player_.draw(window_);
    for (const auto& enemy : enemies_) {
        enemy.draw(window_);
    }

    for (const auto& bullet : bullets_) {
        bullet.draw(window_);
    }

    window_.display();
}

void Game::updateStatus() {
    if (!base_.isAlive() || !player_.isAlive()) {
        status_ = GameStatus::Defeat;
        window_.setTitle("Retro Tank Battle - Defeat");
    } else if (enemies_.empty()) {
        status_ = GameStatus::Victory;
        window_.setTitle("Retro Tank Battle - Victory");
    }
}
