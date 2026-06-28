#include "entity/bullet.hpp"

Bullet::Bullet(sf::Vector2f startPos, Direction dir, Faction faction, int damage, float speed, int ownerId)
    : position_(startPos),
      velocity_(0.f, 0.f),
      damage_(damage),
      ownerId_(ownerId),
      faction_(faction),
      isAlive_(true),
      shape_()
{
    if (damage >= prototype::PlayerBuffDamage) {
        shape_.setFillColor(sf::Color::Red);
        shape_.setRadius(6.f);
        shape_.setOrigin({6.f, 6.f});
    } else {
        shape_.setFillColor(sf::Color::Yellow);
        shape_.setRadius(4.f);
        shape_.setOrigin({4.f, 4.f});
    }
    shape_.setPosition(position_);

    switch (dir) {
    case Direction::Up:
        velocity_ = {0.f, -speed};
        break;
    case Direction::Down:
        velocity_ = {0.f, speed};
        break;
    case Direction::Left:
        velocity_ = {-speed, 0.f};
        break;
    case Direction::Right:
        velocity_ = {speed, 0.f};
        break;
    }
}

void Bullet::update(float deltaTime) {
    if (!isAlive_) {
        return;
    }

    position_ += velocity_ * deltaTime;
    shape_.setPosition(position_);

    if (position_.x < 0.f || position_.x > prototype::WindowWidth ||
        position_.y < 0.f || position_.y > prototype::WindowHeight) {
        isAlive_ = false;
    }
}

void Bullet::draw(sf::RenderWindow& window) const {
    if (isAlive_) {
        window.draw(shape_);
    }
}
