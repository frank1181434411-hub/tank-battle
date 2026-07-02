#include "entity/tank.hpp"

namespace {
int nextTankId = 1;
}

Tank::Tank(float x, float y, float speed, int maxHp, Faction faction, float fireCooldown, float bulletSpeed)
    : position_(x, y),
      dir_(Direction::Up),
      speed_(speed),
      hp_(maxHp),
      maxHp_(maxHp),
      faction_(faction),
      currentBulletDamage_(prototype::PlayerNormalDamage),
      fireCooldown_(fireCooldown),
      cooldownTimer_(0.f),
      bulletSpeed_(bulletSpeed),
      id_(nextTankId++),
      bodyShape_(),
      turretShape_()
{
    bodyShape_.setSize({32.f, 32.f});
    bodyShape_.setOrigin({16.f, 16.f});
    turretShape_.setSize({8.f, 16.f});
    turretShape_.setOrigin({4.f, 16.f});
    updateShapePositions();
}

void Tank::takeDamage(int damage) {
    hp_ -= damage;
    if (hp_ < 0) {
        hp_ = 0;
    }
}

bool Tank::heal(int amount) {
    if (hp_ >= maxHp_) {
        return false;
    }

    hp_ += amount;
    if (hp_ > maxHp_) {
        hp_ = maxHp_;
    }
    return true;
}

void Tank::fire(std::vector<Bullet>& bullets) {
    if (cooldownTimer_ > 0.f) {
        return;
    }

    bullets.emplace_back(position_, dir_, faction_, currentBulletDamage_, bulletSpeed_, id_);
    cooldownTimer_ = fireCooldown_;
}

void Tank::draw(sf::RenderWindow& window) const {
    if (isAlive()) {
        window.draw(bodyShape_);
        window.draw(turretShape_);
    }
}

sf::Vector2f Tank::getPosition() const noexcept
{
    return position_;
}

void Tank::setPosition(sf::Vector2f position)
{
    position_=position;
    updateShapePositions();
}

Direction Tank::getDirection() const noexcept
{
    return dir_;
}

void Tank::setDirection(Direction direction)
{
    dir_=direction;
    updateTurretRotation();
}

void Tank::setHp(int hp) noexcept
{
    hp_=hp;
    if(hp_<0)
        hp_=0;
    if(hp_>maxHp_)
        hp_=maxHp_;
}

sf::FloatRect Tank::boundsAt(sf::Vector2f position) const
{
    sf::FloatRect bounds=getBounds();
    bounds.position+=position-position_;
    return bounds;
}

void Tank::clampToWindow() {
    if (position_.x < prototype::TankHalfSize) {
        position_.x = prototype::TankHalfSize;
    }
    if (position_.x > prototype::WindowWidth - prototype::TankHalfSize) {
        position_.x = prototype::WindowWidth - prototype::TankHalfSize;
    }
    if (position_.y < prototype::TankHalfSize) {
        position_.y = prototype::TankHalfSize;
    }
    if (position_.y > prototype::WindowHeight - prototype::TankHalfSize) {
        position_.y = prototype::WindowHeight - prototype::TankHalfSize;
    }
}

void Tank::updateShapePositions() {
    bodyShape_.setPosition(position_);
    turretShape_.setPosition(position_);
}

void Tank::updateTurretRotation() {
    switch (dir_) {
    case Direction::Up:
        turretShape_.setRotation(sf::degrees(0.f));
        break;
    case Direction::Down:
        turretShape_.setRotation(sf::degrees(180.f));
        break;
    case Direction::Left:
        turretShape_.setRotation(sf::degrees(270.f));
        break;
    case Direction::Right:
        turretShape_.setRotation(sf::degrees(90.f));
        break;
    }
}
