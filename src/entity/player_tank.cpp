#include "entity/player_tank.hpp"

PlayerTank::PlayerTank(float x, float y)
    : Tank(x, y, 120.f, 100, Faction::Player, prototype::PlayerFireCooldown, prototype::PlayerBulletSpeed),
      damageBuffTimer_(0.f)
{
    bodyShape_.setFillColor(sf::Color::Green);
    turretShape_.setFillColor(sf::Color::White);
}

void PlayerTank::update(float deltaTime, std::vector<Bullet>& bullets) {
    if (!isAlive()) {
        return;
    }

    if (cooldownTimer_ > 0.f) {
        cooldownTimer_ -= deltaTime;
    }

    if (damageBuffTimer_ > 0.f) {
        damageBuffTimer_ -= deltaTime;
        if (damageBuffTimer_ <= 0.f) {
            currentBulletDamage_ = prototype::PlayerNormalDamage;
            turretShape_.setFillColor(sf::Color::White);
        }
    }

    handleMovement(deltaTime);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        int activePlayerBullets = 0;
        for (const auto& bullet : bullets) {
            if (bullet.getFaction() == Faction::Player && bullet.getOwnerId() == getId()) {
                ++activePlayerBullets;
            }
        }
        if (activePlayerBullets < 3) {
            fire(bullets);
        }
    }
}

void PlayerTank::handleMovement(float deltaTime) {
    sf::Vector2f movement(0.f, 0.f);
    bool moved = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        dir_ = Direction::Up;
        movement.y -= speed_ * deltaTime;
        moved = true;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        dir_ = Direction::Down;
        movement.y += speed_ * deltaTime;
        moved = true;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        dir_ = Direction::Left;
        movement.x -= speed_ * deltaTime;
        moved = true;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        dir_ = Direction::Right;
        movement.x += speed_ * deltaTime;
        moved = true;
    }

    if (moved) {
        position_ += movement;
        clampToWindow();
        updateShapePositions();
    }
    updateTurretRotation();
}

void PlayerTank::collectItem(Item& item) {
    if (item.getType() == ItemType::HealthPack) {
        if (heal(40)) {
            item.pickUp();
        }
    } else if (item.getType() == ItemType::DamagePack) {
        currentBulletDamage_ = prototype::PlayerBuffDamage;
        damageBuffTimer_ = 15.f;
        turretShape_.setFillColor(sf::Color::Yellow);
        item.pickUp();
    }
}
