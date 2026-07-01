#pragma once

#include <SFML/System/Vector2.hpp>

#include<cstddef>
#include<cstdint>
#include<random>
#include<vector>

#include"ai/ai_context.hpp"
#include"ai/ai_controller.hpp"

struct GridPoint
{
    int x=0,y=0;
};

class EasyAI final:public AIController
{
public:
    explicit EasyAI(std::uint32_t seed);
    TankCommand decide(const EnemyTank& self,const AIContext& context,float deltaTime) override;
    void reset();

private:
    TankCommand makeDecision(const EnemyTank& self,const AIContext& context);
    bool updateStuckState(const EnemyTank& self,float deltaTime);
    Direction chooseRandomDirection(Direction currentDirection);
    bool randomChance(float probability);
    static constexpr float DecisionInterval=0.65f;
    static constexpr float StuckTimeout=0.25f;
    static constexpr float TurnProbability=0.35f;
    static constexpr float FireProbability=0.18f;
    float decisionTimer_=0.f;
    float stuckTimer_=0.f;
    TankCommand currentCommand_{};
    sf::Vector2f lastPosition_{0.f,0.f};
    std::mt19937 random_;
    bool positionInitialized_=false;
};

class NormalAI final:public AIController
{
public:
    explicit NormalAI(std::uint32_t seed);
    TankCommand decide(const EnemyTank& self,const AIContext& context,float deltaTime) override;
    void reset();

private:
    enum class State{Patrol,Chase,Attack};
    TankCommand makeDecision(const EnemyTank& self,const AIContext& context);
    sf::Vector2f selectTargetPosition(const EnemyTank& self,const AIContext& context);
    void rebuildPath(const EnemyTank& self,const AIContext& context);
    Direction followPath(const EnemyTank& self,const AIContext& context);
    bool updateStuckState(const EnemyTank& self,float deltaTime);
    Direction chooseFallbackDirection(Direction currentDirection);
    bool randomChance(float probability);
    void clearPath() noexcept;
    static constexpr float DecisionInterval=0.35f;
    static constexpr float PathRefreshInterval=0.6f;
    static constexpr float StuckTimeout=0.25f;
    static constexpr float FireProbability=0.3f;
    State state_=State::Patrol;
    float decisionTimer_=0.f;
    float pathRefreshTimer_=0.f;
    float stuckTimer_=0.f;
    TankCommand currentCommand_{};
    sf::Vector2f lastPosition_{0.f,0.f};
    sf::Vector2f targetPosition_{0.f,0.f};
    std::vector<GridPoint> path_;
    std::mt19937 random_;
    bool positionInitialized_=false;
};

class HardAI final:public AIController
{
public:
    explicit HardAI(std::uint32_t seed);
    TankCommand decide(const EnemyTank& self,const AIContext& context,float deltaTime) override;
    void reset();

private:
    enum class State{ChasePlayer,AttackPlayer,AttackBase,BreakWall};
    enum class TargetType{None,Player,Base};
    struct TargetInfo
    {
        TargetType type=TargetType::None;
        sf::Vector2f position{0.f,0.f};
        float score=0.f;
    };
    TankCommand makeDecision(const EnemyTank& self,const AIContext& context);
    TargetInfo selectTarget(const EnemyTank& self,const AIContext& context) const;
    float calculatePlayerScore(const EnemyTank& self,sf::Vector2f playerPosition,int playerHp) const noexcept;
    float calculateBaseScore(const EnemyTank& self,sf::Vector2f basePosition) const noexcept;
    void rebuildPath(const EnemyTank& self,const AIContext& context,sf::Vector2f targetPosition);
    Direction followPath(const EnemyTank& self,const AIContext& context);
    bool shouldAttackBrick(const EnemyTank& self,const AIContext& context,Direction direction) const;
    bool updateStuckState(const EnemyTank& self,float dataTime);
    Direction chooseFallbackDirection(Direction currentDirection);
    bool randomChance(float probability);
    void clearPath() noexcept;
    static constexpr float DecisionInterval=0.15f;
    static constexpr float PathRefreshInterval=0.3f;
    static constexpr float StuckTimeout=0.18f;
    static constexpr float FireProbability=0.5f;
    State state_=State::ChasePlayer;
    float decisionTimer_=0.f;
    float pathRefreshTimer_=0.f;
    float stuckTimer_=0.f;
    TankCommand currentCommand_{};
    TargetInfo currentTarget_{};
    sf::Vector2f lastPosition_{0.f,0.f};
    std::vector<GridPoint> path_;
    std::size_t pathIndex_=0;
    std::mt19937 random_;
    bool positionInitialized_=false;
};
