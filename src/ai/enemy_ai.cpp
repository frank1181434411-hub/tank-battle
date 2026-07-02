#include"ai/enemy_ai.hpp"
#include"world/collision.hpp"
#include"world/tile.hpp"

#include<algorithm>
#include<array>
#include<cmath>
#include<cstddef>
#include<limits>
#include<queue>
#include<random>
#include<vector>

namespace
{
constexpr float PositionEpsilonSquared=0.01f;
constexpr std::array<Direction,4> AllDirections{Direction::Up,Direction::Down,Direction::Left,Direction::Right};

struct ShotCell
{
    GridPoint point;
    int weight=0;
};

Direction randomDirection(std::mt19937& random)
{
    std::uniform_int_distribution<std::size_t> distribution(0,AllDirections.size()-1);
    return AllDirections[distribution(random)];
}

GridPoint worldToGrid(sf::Vector2f position){return {static_cast<int>(std::floor(position.x/Tile::DefaultSize)),static_cast<int>(std::floor(position.y/Tile::DefaultSize))};}

int gridIndex(const GridPoint& point,int mapWidth) noexcept{return point.y*mapWidth+point.x;}

bool isWalkable(const EnemyTank& self,const Map& map,const GridPoint& point)
{
    if(!map.isInBounds(point.x,point.y)) return 0;
    const sf::Vector2f position=map.tileCenter(point.x,point.y);
    const sf::FloatRect bounds=self.boundsAt(position);
    return !Collision::hitsTankBlock(map,bounds);
}

std::vector<ShotCell> collectShotCells(const EnemyTank& self,const Map& map,sf::Vector2f targetPosition)
{
    const GridPoint target=worldToGrid(targetPosition);
    constexpr std::array<GridPoint,4> offsets{GridPoint{0,-1},GridPoint{0,1},GridPoint{-1,0},GridPoint{1,0}};
    std::vector<ShotCell> cells;
    for(const GridPoint& offset:offsets)
    {
        int weight=2;
        GridPoint current{target.x+offset.x,target.y+offset.y};
        while(map.isInBounds(current.x,current.y))
        {
            const Tile* tile=map.tileAt(current.x,current.y);
            if(tile==nullptr) break;
            if(tile->type()==TileType::Steel) break;
            if(tile->type()==TileType::Brick) weight=1;
            else if(isWalkable(self,map,current)) cells.push_back({current,weight});
            current.x+=offset.x;current.y+=offset.y;
        }
    }
    return cells;
}

Direction directionTo(sf::Vector2f start,sf::Vector2f target) noexcept
{
    const float deltaX=target.x-start.x;
    const float deltaY=target.y-start.y;
    if(std::abs(deltaX)>std::abs(deltaY)) return deltaX>0.f?Direction::Right:Direction::Left;
    return deltaY>0.f?Direction::Down:Direction::Up;
}

bool findClearShotDirection(const EnemyTank& self,sf::Vector2f targetPosition,const Map& map,Direction& result)
{
    constexpr float AlignmentTolerance=18.f;
    const sf::Vector2f start=self.getPosition();
    const float deltaX=targetPosition.x-start.x;
    const float deltaY=targetPosition.y-start.y;
    if(std::abs(deltaX)<=AlignmentTolerance) result=deltaY>0.f?Direction::Down:Direction::Up;
    else if(std::abs(deltaY)<=AlignmentTolerance) result=deltaX>0.f?Direction::Right:Direction::Left;
    else return 0;
    const GridPoint startGrid=worldToGrid(start);
    const GridPoint targetGrid=worldToGrid(targetPosition);
    if(startGrid.x==targetGrid.x)
    {
        const int begin=std::min(startGrid.y,targetGrid.y)+1;
        const int end=std::max(startGrid.y,targetGrid.y);
        for(int y=begin;y<end;++y)
        {
            const Tile* tile=map.tileAt(startGrid.x,y);
            if(tile==nullptr || tile->blocksBullet()) return 0;
        }
        return 1;
    }
    const int begin=std::min(startGrid.x,targetGrid.x)+1;
    const int end=std::max(startGrid.x,targetGrid.x);
    for(int x=begin;x<end;++x)
    {
        const Tile* tile=map.tileAt(x,startGrid.y);
        if(tile==nullptr || tile->blocksBullet()) return 0;
    }
    return 1;
}

std::vector<GridPoint> findPath(const EnemyTank& self,const Map& map,sf::Vector2f targetPosition)
{
    const GridPoint start=worldToGrid(self.getPosition());
    const GridPoint goal=worldToGrid(targetPosition);

    if(!map.isInBounds(start.x,start.y) || !map.isInBounds(goal.x,goal.y))return {};
    constexpr std::array<GridPoint,4> offsets{GridPoint{0,-1},GridPoint{0,1},GridPoint{-1,0},GridPoint{1,0}};
    const int nodeCount=map.width()*map.height();
    if(nodeCount<=0) return {};
    std::queue<GridPoint> open;
    std::vector<bool> visited(static_cast<std::size_t>(nodeCount),false);
    std::vector<int> parent(static_cast<std::size_t>(nodeCount),-1);
    const int startIndex=gridIndex(start,map.width());
    visited[static_cast<std::size_t>(startIndex)]=true;
    open.push(start);
    while(!open.empty())
    {
        const GridPoint current=open.front();
        open.pop();
        if(current==goal) break;

        for(const GridPoint& offset:offsets)
        {
            const GridPoint next{current.x+offset.x,current.y+offset.y};
            if(!map.isInBounds(next.x,next.y)) continue;

            const int nextIndex=gridIndex(next,map.width());
            if(visited[static_cast<std::size_t>(nextIndex)]) continue;
            if(!isWalkable(self,map,next)) continue;

            visited[static_cast<std::size_t>(nextIndex)]=true;
            parent[static_cast<std::size_t>(nextIndex)]=gridIndex(current,map.width());
            open.push(next);
        }
    }
    const int goalIndex=gridIndex(goal,map.width());
    if(!visited[static_cast<std::size_t>(goalIndex)])
        return {};

    std::vector<GridPoint> path;
    int currentIndex=goalIndex;
    while(currentIndex!=-1)
    {
        const GridPoint point{currentIndex%map.width(),currentIndex/map.width()};
        path.push_back(point);
        if(currentIndex==startIndex) break;
        currentIndex=parent[static_cast<std::size_t>(currentIndex)];
    }

    std::reverse(path.begin(),path.end());
    return path;
}

std::vector<GridPoint> findPathToBestShotCell(const EnemyTank& self,const Map& map,sf::Vector2f targetPosition,int minimumWeight)
{
    const std::vector<ShotCell> shotCells=collectShotCells(self,map,targetPosition);
    std::vector<GridPoint> bestPath;
    std::vector<GridPoint> bestDirectPath;
    std::vector<GridPoint> bestBrickPath;
    constexpr std::size_t BrickPathExtraCost=8;

    for(const ShotCell& shotCell:shotCells)
    {
        if(shotCell.weight<minimumWeight)
            continue;

        std::vector<GridPoint> path=findPath(self,map,map.tileCenter(shotCell.point.x,shotCell.point.y));
        if(path.empty())
            continue;

        if(bestPath.empty() || path.size()<bestPath.size())
            bestPath=path;

        if(shotCell.weight==2 && (bestDirectPath.empty() || path.size()<bestDirectPath.size()))
            bestDirectPath=path;

        if(shotCell.weight==1 && (bestBrickPath.empty() || path.size()<bestBrickPath.size()))
            bestBrickPath=path;
    }

    if(minimumWeight<=1 && !bestDirectPath.empty())
    {
        if(bestBrickPath.empty() || bestDirectPath.size()<=bestBrickPath.size()+BrickPathExtraCost)
            return bestDirectPath;
    }

    return bestPath;
}
}

// easy ai

EasyAI::EasyAI(std::uint32_t seed):random_(seed){currentCommand_.direction=randomDirection(random_);}

TankCommand EasyAI::decide(const EnemyTank& self,const AIContext& context,float deltaTime)
{
    if(!self.isAlive()) return {};
    const bool stuck=updateStuckState(self,deltaTime);
    decisionTimer_-=deltaTime;
    if(!stuck && decisionTimer_>0.f)
    {
        TankCommand command=currentCommand_;
        command.fire=false;
        return command;
    }
    decisionTimer_=DecisionInterval;
    if(stuck)
    {
        currentCommand_.direction=chooseRandomDirection(currentCommand_.direction);
        currentCommand_.move=true;
        currentCommand_.fire=randomChance(FireProbability);
        return currentCommand_;
    }
    currentCommand_=makeDecision(self,context);
    return currentCommand_;
}

TankCommand EasyAI::makeDecision(const EnemyTank& self,const AIContext& context)
{
    (void)self;
    (void)context;
    TankCommand command=currentCommand_;
    if(command.move && randomChance(TurnProbability)) command.direction=chooseRandomDirection(command.direction);
    command.move=true;
    command.fire=randomChance(FireProbability);
    return command;
}

bool EasyAI::updateStuckState(const EnemyTank& self,float deltaTime)
{
    const sf::Vector2f currentPosition=self.getPosition();
    if(!positionInitialized_)
    {
        lastPosition_=currentPosition;
        positionInitialized_=true;
        stuckTimer_=0.f;
        return 0;
    }
    const float deltaX=currentPosition.x-lastPosition_.x;
    const float deltaY=currentPosition.y-lastPosition_.y;
    const float distanceSquared=deltaX*deltaX+deltaY*deltaY;
    lastPosition_=currentPosition;
    if(!currentCommand_.move || distanceSquared>PositionEpsilonSquared)
    {
        stuckTimer_=0.f;
        return 0;
    }
    stuckTimer_+=deltaTime;
    if(stuckTimer_<StuckTimeout) return 0;
    stuckTimer_=0.f;
    return 1;
}

Direction EasyAI::chooseRandomDirection(Direction currentDirection)
{
    std::array<Direction,3> candidates{};
    std::size_t count=0;
    for(Direction direction:AllDirections)
    {
        if(direction==currentDirection) continue;
        candidates[count]=direction;
        ++count;
    }
    std::uniform_int_distribution<std::size_t> distribution(0,count-1);
    return candidates[distribution(random_)];
}

bool EasyAI::randomChance(float probability)
{
    if(probability<=0.f) return 0;
    if(probability>=1.f) return 1;
    std::bernoulli_distribution distribution(probability);
    return distribution(random_);
}

void EasyAI::reset()
{
    decisionTimer_=0.f;
    stuckTimer_=0.f;
    currentCommand_={};
    currentCommand_.direction=randomDirection(random_);
    lastPosition_={0.f,0.f};
    positionInitialized_=false;
}

// normal ai

NormalAI::NormalAI(std::uint32_t seed):random_(seed)
{
    currentCommand_.direction=randomDirection(random_);
}

TankCommand NormalAI::decide(const EnemyTank& self,const AIContext& context,float deltaTime)
{
    if(!self.isAlive()) return {};

    const bool stuck=updateStuckState(self,deltaTime);
    decisionTimer_-=deltaTime;
    pathRefreshTimer_-=deltaTime;

    if(stuck)
    {
        clearPath();
        decisionTimer_=0.f;
        pathRefreshTimer_=0.f;
    }

    if(decisionTimer_>0.f)
    {
        TankCommand command=currentCommand_;
        command.fire=false;
        return command;
    }

    decisionTimer_=DecisionInterval;
    currentCommand_=makeDecision(self,context);
    return currentCommand_;
}

TankCommand NormalAI::makeDecision(const EnemyTank& self,const AIContext& context)
{
    const sf::Vector2f target=selectTargetPosition(self,context);
    Direction attackDirection=self.getDirection();

    if(findClearShotDirection(self,target,context.map,attackDirection))
    {
        state_=State::Attack;
        return {attackDirection,false,true};
    }

    state_=State::Chase;

    if(pathRefreshTimer_<=0.f || path_.empty() || pathIndex_>=path_.size())
        rebuildPath(self,context,target);

    if(!path_.empty() && pathIndex_<path_.size())
        return {followPath(self,context),true,false};

    state_=State::Patrol;
    return {chooseFallbackDirection(self.getDirection()),true,false};
}

sf::Vector2f NormalAI::selectTargetPosition(const EnemyTank& self,const AIContext& context) const
{
    const PlayerTank* targetPlayer=nullptr;
    float minimumDistanceSquared=std::numeric_limits<float>::max();

    for(const PlayerTank& player:context.players)
    {
        if(!player.isAlive()) continue;

        const sf::Vector2f delta=player.getPosition()-self.getPosition();
        const float distanceSquared=delta.x*delta.x+delta.y*delta.y;

        if(distanceSquared>=minimumDistanceSquared) continue;

        minimumDistanceSquared=distanceSquared;
        targetPlayer=&player;
    }

    if(targetPlayer!=nullptr)
        return targetPlayer->getPosition();

    const sf::FloatRect baseBounds=context.base.getBounds();
    return baseBounds.position+baseBounds.size*0.5f;
}

void NormalAI::rebuildPath(const EnemyTank& self,const AIContext& context,sf::Vector2f targetPosition)
{
    targetPosition_=targetPosition;
    path_=findPathToBestShotCell(self,context.map,targetPosition_,2);

    if(path_.size()>1)
        pathIndex_=1;
    else
        pathIndex_=path_.size();

    pathRefreshTimer_=PathRefreshInterval;
}

Direction NormalAI::followPath(const EnemyTank& self,const AIContext& context)
{
    constexpr float ReachDistanceSquared=25.f;

    while(pathIndex_<path_.size())
    {
        const GridPoint& point=path_[pathIndex_];
        const sf::Vector2f nextPosition=context.map.tileCenter(point.x,point.y);
        const sf::Vector2f delta=nextPosition-self.getPosition();
        const float distanceSquared=delta.x*delta.x+delta.y*delta.y;

        if(distanceSquared>ReachDistanceSquared)
            return directionTo(self.getPosition(),nextPosition);

        ++pathIndex_;
    }

    return self.getDirection();
}

bool NormalAI::updateStuckState(const EnemyTank& self,float deltaTime)
{
    const sf::Vector2f currentPosition=self.getPosition();

    if(!positionInitialized_)
    {
        lastPosition_=currentPosition;
        positionInitialized_=true;
        stuckTimer_=0.f;
        return 0;
    }

    const float deltaX=currentPosition.x-lastPosition_.x;
    const float deltaY=currentPosition.y-lastPosition_.y;
    const float distanceSquared=deltaX*deltaX+deltaY*deltaY;
    lastPosition_=currentPosition;

    if(!currentCommand_.move || distanceSquared>PositionEpsilonSquared)
    {
        stuckTimer_=0.f;
        return 0;
    }

    stuckTimer_+=deltaTime;

    if(stuckTimer_<StuckTimeout)
        return 0;

    stuckTimer_=0.f;
    return 1;
}

Direction NormalAI::chooseFallbackDirection(Direction currentDirection)
{
    std::array<Direction,3> candidates{};
    std::size_t count=0;

    for(Direction direction:AllDirections)
    {
        if(direction==currentDirection) continue;
        candidates[count]=direction;
        ++count;
    }

    std::uniform_int_distribution<std::size_t> distribution(0,count-1);
    return candidates[distribution(random_)];
}

bool NormalAI::randomChance(float probability)
{
    if(probability<=0.f) return 0;
    if(probability>=1.f) return 1;
    std::bernoulli_distribution distribution(probability);
    return distribution(random_);
}

void NormalAI::clearPath() noexcept
{
    path_.clear();
    pathIndex_=0;
    pathRefreshTimer_=0.f;
}

void NormalAI::reset()
{
    state_=State::Patrol;
    decisionTimer_=0.f;
    pathRefreshTimer_=0.f;
    stuckTimer_=0.f;
    currentCommand_={};
    currentCommand_.direction=randomDirection(random_);
    lastPosition_={0.f,0.f};
    targetPosition_={0.f,0.f};
    clearPath();
    positionInitialized_=false;
}

// hard ai

HardAI::HardAI(std::uint32_t seed):random_(seed){currentCommand_.direction=randomDirection(random_);}

TankCommand HardAI::decide(const EnemyTank& self,const AIContext& context,float deltaTime)
{
    if(!self.isAlive()) return {};
    const bool stuck=updateStuckState(self,deltaTime);
    decisionTimer_-=deltaTime;
    pathRefreshTimer_-=deltaTime;
    if(stuck)
    {
        clearPath();
        currentCommand_.direction=chooseFallbackDirection(self.getDirection());
        currentCommand_.move=true;
        currentCommand_.fire=false;
        decisionTimer_=DecisionInterval;
        return currentCommand_;
    }
    if(decisionTimer_>0.f)
    {
        TankCommand command=currentCommand_;
        command.fire=false;
        return command;
    }
    decisionTimer_=DecisionInterval;
    currentCommand_=makeDecision(self,context);
    return currentCommand_;
}

TankCommand HardAI::makeDecision(const EnemyTank& self,const AIContext& context)
{
    const TargetInfo selectedTarget=selectTarget(self,context);
    if(selectedTarget.type==TargetType::None)
    {
        clearPath();
        state_=State::ChasePlayer;
        return {chooseFallbackDirection(self.getDirection()),true,randomChance(FireProbability*0.1f)};
    }
    const sf::Vector2f targetDifference=selectedTarget.position-currentTarget_.position;
    const float targetChangeSquared=targetDifference.x*targetDifference.x+targetDifference.y*targetDifference.y;
    const bool targetChanged=selectedTarget.type!=currentTarget_.type || targetChangeSquared>=Tile::DefaultSize*Tile::DefaultSize;
    currentTarget_=selectedTarget;

    if(targetChanged)
        clearPath();

    Direction attackDirection=self.getDirection();
    if(findClearShotDirection(self,currentTarget_.position,context.map,attackDirection))
    {
        clearPath();
        if(currentTarget_.type==TargetType::Player) state_=State::AttackPlayer;
        else state_=State::AttackBase;
        return {attackDirection,false,true};
    }
    const sf::Vector2f targetDelta=currentTarget_.position-self.getPosition();
    constexpr float AlignmentTolerance=18.f;
    const bool aligned=std::abs(targetDelta.x)<=AlignmentTolerance || std::abs(targetDelta.y)<=AlignmentTolerance;
    if(aligned && shouldAttackBrick(self,context,attackDirection))
    {
        state_=State::BreakWall;
        clearPath();
        return {attackDirection,false,true};
    }
    if(pathRefreshTimer_<=0.f || path_.empty() || pathIndex_>=path_.size()) rebuildPath(self,context,currentTarget_.position);
    if(!path_.empty() && pathIndex_<path_.size())
    {
        const Direction moveDirection=followPath(self,context);
        if(shouldAttackBrick(self,context,moveDirection))
        {
            state_=State::BreakWall;
            return {moveDirection,false,true};
        }
        if(currentTarget_.type==TargetType::Player) state_=State::ChasePlayer;
        else state_=State::AttackBase;
        return {moveDirection,true,randomChance(FireProbability*0.12f)};
    }
    if(currentTarget_.type==TargetType::Player) state_=State::ChasePlayer;
    else state_=State::AttackBase;
    return {chooseFallbackDirection(self.getDirection()),true,randomChance(FireProbability*0.1f)};
}

HardAI::TargetInfo HardAI::selectTarget(const EnemyTank& self,const AIContext& context) const
{
    TargetInfo bestTarget{};
    const GridPoint selfGrid=worldToGrid(self.getPosition());
    const int maximumGridDistance=std::max(1,context.map.width()+context.map.height());
    for(const PlayerTank& player:context.players)
    {
        if(!player.isAlive()) continue;
        float score=calculatePlayerScore(self,player.getPosition(),player.getHp());
        const GridPoint playerGrid=worldToGrid(player.getPosition());
        const int gridDistance=std::abs(playerGrid.x-selfGrid.x)+std::abs(playerGrid.y-selfGrid.y);
        const float pathScore=std::clamp(1.f-static_cast<float>(gridDistance)/static_cast<float>(maximumGridDistance),0.f,1.f);
        score+=0.2f*pathScore;
        Direction shotDirection=self.getDirection();
        if(findClearShotDirection(self,player.getPosition(),context.map,shotDirection)) score+=0.25f;
        if(bestTarget.type!=TargetType::None && score<=bestTarget.score) continue;
        bestTarget.type=TargetType::Player;
        bestTarget.position=player.getPosition();
        bestTarget.score=score;
    }
    if(context.base.isAlive())
    {
        const sf::FloatRect baseBounds=context.base.getBounds();
        const sf::Vector2f basePosition=baseBounds.position+baseBounds.size*0.5f;
        float score=calculateBaseScore(self,basePosition);
        const GridPoint baseGrid=worldToGrid(basePosition);
        const int gridDistance=std::abs(baseGrid.x-selfGrid.x)+std::abs(baseGrid.y-selfGrid.y);
        const float pathScore=std::clamp(1.f-static_cast<float>(gridDistance)/static_cast<float>(maximumGridDistance),0.f,1.f);
        score+=0.2f*pathScore;
        Direction shotDirection=self.getDirection();
        if(findClearShotDirection(self,basePosition,context.map,shotDirection)) score+=0.2f;
        if(bestTarget.type==TargetType::None || score>bestTarget.score)
        {
            bestTarget.type=TargetType::Base;
            bestTarget.position=basePosition;
            bestTarget.score=score;
        }
    }
    return bestTarget;
}

float HardAI::calculatePlayerScore(const EnemyTank& self,sf::Vector2f playerPosition,int playerHp) const noexcept
{
    constexpr float MaximumReferenceDistance=800.f;
    constexpr float PlayerMaximumHp=100.f;
    const sf::Vector2f delta=playerPosition-self.getPosition();
    const float distance=std::sqrt(delta.x*delta.x+delta.y*delta.y);
    const float distanceScore=std::clamp(1.f-distance/MaximumReferenceDistance,0.f,1.f);
    const float lowHealthScore=std::clamp(1.f-static_cast<float>(playerHp)/PlayerMaximumHp,0.f,1.f);
    return 0.3f*distanceScore+0.25f*lowHealthScore;
}

float HardAI::calculateBaseScore(const EnemyTank& self,sf::Vector2f basePosition) const noexcept
{
    constexpr float MaximumReferenceDistance=800.f;
    constexpr float BaseStrategicScore=1.f;
    const sf::Vector2f delta=basePosition-self.getPosition();
    const float distance=std::sqrt(delta.x*delta.x+delta.y*delta.y);
    const float distanceScore=std::clamp(1.f-distance/MaximumReferenceDistance,0.f,1.f);
    return 0.3f*distanceScore+0.3f*BaseStrategicScore;
}

void HardAI::rebuildPath(const EnemyTank& self,const AIContext& context,sf::Vector2f targetPosition)
{
    path_=findPathToBestShotCell(self,context.map,targetPosition,1);
    if(path_.size()>1) pathIndex_=1;
    else pathIndex_=path_.size();
    pathRefreshTimer_=PathRefreshInterval;
}
Direction HardAI::followPath(const EnemyTank& self,const AIContext& context)
{
    constexpr float ReachDistanceSquared=25.f;
    while(pathIndex_<path_.size())
    {
        const GridPoint& point=path_[pathIndex_];
        const sf::Vector2f nextPosition=context.map.tileCenter(point.x,point.y);
        const sf::Vector2f delta=nextPosition-self.getPosition();
        const float distanceSquared=delta.x*delta.x+delta.y*delta.y;
        if(distanceSquared>ReachDistanceSquared) return directionTo(self.getPosition(),nextPosition);
        ++pathIndex_;
    }
    return self.getDirection();
}

bool HardAI::shouldAttackBrick(const EnemyTank& self,const AIContext& context,Direction direction) const
{
    if(currentTarget_.type==TargetType::None) return 0;
    const GridPoint start=worldToGrid(self.getPosition());
    const GridPoint target=worldToGrid(currentTarget_.position);
    GridPoint step{};
    if(direction==Direction::Up)
    {
        if(target.x!=start.x || target.y>=start.y) return 0;
        step={0,-1};
    }
    else if(direction==Direction::Down)
    {
        if(target.x!=start.x || target.y<=start.y) return 0;
        step={0,1};
    }
    else if(direction==Direction::Left)
    {
        if(target.y!=start.y || target.x>=start.x) return 0;
        step={-1,0};
    }
    else
    {
        if(target.y!=start.y || target.x<=start.x) return 0;
        step={1,0};
    }

    GridPoint current{start.x+step.x,start.y+step.y};
    while(context.map.isInBounds(current.x,current.y))
    {
        const Tile* tile=context.map.tileAt(current.x,current.y);
        if(tile==nullptr) return 0;
        if(tile->type()==TileType::Brick) return 1;
        if(tile->type()==TileType::Steel) return 0;
        if(current==target) return 0;
        current.x+=step.x;
        current.y+=step.y;
    }
    return 0;
}

bool HardAI::updateStuckState(const EnemyTank& self,float deltaTime)
{
    const sf::Vector2f currentPosition=self.getPosition();
    if(!positionInitialized_)
    {
        lastPosition_=currentPosition;
        positionInitialized_=true;
        stuckTimer_=0.f;
        return 0;
    }
    const float deltaX=currentPosition.x-lastPosition_.x;
    const float deltaY=currentPosition.y-lastPosition_.y;
    const float distanceSquared=deltaX*deltaX+deltaY*deltaY;
    lastPosition_=currentPosition;
    if(!currentCommand_.move || distanceSquared>PositionEpsilonSquared)
    {
        stuckTimer_=0.f;
        return 0;
    }
    stuckTimer_+=deltaTime;
    if(stuckTimer_<StuckTimeout) return 0;
    stuckTimer_=0.f;
    return 1;
}

Direction HardAI::chooseFallbackDirection(Direction currentDirection)
{
    std::array<Direction,3> candidates{};
    std::size_t count=0;
    for(Direction direction:AllDirections)
    {
        if(direction==currentDirection) continue;
        candidates[count]=direction;
        ++count;
    }
    std::uniform_int_distribution<std::size_t> distribution(0,count-1);
    return candidates[distribution(random_)];
}

bool HardAI::randomChance(float probability)
{
    if(probability<=0.f) return 0;
    if(probability>=1.f) return 1;
    std::bernoulli_distribution distribution(probability);
    return distribution(random_);
}

void HardAI::clearPath() noexcept
{
    path_.clear();
    pathIndex_=0;
    pathRefreshTimer_=0.f;
}

void HardAI::reset()
{
    state_=State::ChasePlayer;
    decisionTimer_=0.f;
    pathRefreshTimer_=0.f;
    stuckTimer_=0.f;
    currentCommand_={};
    currentCommand_.direction=randomDirection(random_);
    currentTarget_={};
    lastPosition_={0.f,0.f};
    clearPath();
    positionInitialized_=false;
}
