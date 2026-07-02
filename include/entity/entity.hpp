#pragma once

namespace prototype {
constexpr float WindowWidth = 800.f;
constexpr float WindowHeight = 600.f;
constexpr float TankHalfSize = 18.f;
constexpr float PlayerBulletSpeed = 320.f;
constexpr float LightEnemyBulletSpeed = 290.f;
constexpr float HeavyEnemyBulletSpeed = 270.f;
constexpr int PlayerNormalDamage = 20;
constexpr int PlayerBuffDamage = 30;
constexpr float PlayerFireCooldown = 0.45f;
constexpr float LightEnemyFireCooldown = 1.1f;
constexpr float HeavyEnemyFireCooldown = 1.8f;
}

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

enum class Faction {
    Player,
    Enemy
};
