#include "entity/enemy_tank.hpp"
#include "world/collision.hpp"

namespace {
bool hitsTankBlock(const sf::FloatRect& bounds,const std::vector<sf::FloatRect>& tankBlocks) noexcept
{
    for(const auto& block:tankBlocks)
    {
        if(bounds.findIntersection(block).has_value())
            return true;
    }

    return false;
}

float enemySpeed(EnemyType type) {
    return type == EnemyType::Light ? 120.f : 70.f;
}

int enemyHealth(EnemyType type) {
    return type == EnemyType::Light ? 50 : 120;
}

float enemyCooldown(EnemyType type) {
    return type == EnemyType::Light ? prototype::LightEnemyFireCooldown : prototype::HeavyEnemyFireCooldown;
}

float enemyBulletSpeed(EnemyType type) {
    return type == EnemyType::Light ? prototype::LightEnemyBulletSpeed : prototype::HeavyEnemyBulletSpeed;
}
}

EnemyTank::EnemyTank(float x, float y, EnemyType type)
    : Tank(x, y, enemySpeed(type), enemyHealth(type), Faction::Enemy, enemyCooldown(type), enemyBulletSpeed(type)),
      type_(type),
      isMoving_(false)
{
    if (type == EnemyType::Light) {
        bodyShape_.setFillColor(sf::Color(255, 165, 0));
        currentBulletDamage_ = 15;
    } else {
        bodyShape_.setFillColor(sf::Color(139, 0, 0));
        currentBulletDamage_ = 25;
    }
    turretShape_.setFillColor(sf::Color::White);
}

void EnemyTank::update(float deltaTime, std::vector<Bullet>& bullets) {
    (void)bullets;
    if (!isAlive()) {
        return;
    }

    if (cooldownTimer_ > 0.f) {
        cooldownTimer_ -= deltaTime;
    }

    if (isMoving_) {
        sf::Vector2f movement(0.f, 0.f);
        if (dir_ == Direction::Up) {
            movement.y -= speed_ * deltaTime;
        } else if (dir_ == Direction::Down) {
            movement.y += speed_ * deltaTime;
        } else if (dir_ == Direction::Left) {
            movement.x -= speed_ * deltaTime;
        } else if (dir_ == Direction::Right) {
            movement.x += speed_ * deltaTime;
        }

        position_ += movement;
        clampToWindow();
        updateShapePositions();
    }

    updateTurretRotation();
}

void EnemyTank::update(float deltaTime,std::vector<Bullet>& bullets,const Map& map,const std::vector<sf::FloatRect>& tankBlocks)
{
    (void)bullets;
    if(!isAlive()) return;

    if(cooldownTimer_>0.f)
        cooldownTimer_-=deltaTime;

    if(isMoving_)
    {
        const sf::Vector2f nextPosition=position_+movementFor(deltaTime);
        const sf::FloatRect nextBounds=boundsAt(nextPosition);
        if(!Collision::hitsTankBlock(map,nextBounds) && !hitsTankBlock(nextBounds,tankBlocks))
            setPosition(nextPosition);
    }

    updateTurretRotation();
}

void EnemyTank::setMoveDirection(Direction dir) {
    dir_ = dir;
    isMoving_ = true;
}

void EnemyTank::stopMoving() {
    isMoving_ = false;
}

void EnemyTank::tryFire(std::vector<Bullet>& bullets) {
    int activeOwnBullets = 0;
    for (const auto& bullet : bullets) {
        if (bullet.getFaction() == Faction::Enemy && bullet.getOwnerId() == getId()) {
            ++activeOwnBullets;
        }
    }

    const int maxBullets = type_ == EnemyType::Light ? 1 : 2;
    if (activeOwnBullets < maxBullets) {
        fire(bullets);
    }
}

sf::Vector2f EnemyTank::movementFor(float deltaTime) const
{
    sf::Vector2f movement(0.f,0.f);
    if(dir_==Direction::Up)
        movement.y-=speed_*deltaTime;
    else if(dir_==Direction::Down)
        movement.y+=speed_*deltaTime;
    else if(dir_==Direction::Left)
        movement.x-=speed_*deltaTime;
    else if(dir_==Direction::Right)
        movement.x+=speed_*deltaTime;

    return movement;
}
