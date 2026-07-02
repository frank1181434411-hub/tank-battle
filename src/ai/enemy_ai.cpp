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
    if(!map.isInBounds(point.x,point.y)) return false;
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
    else return false;
    const GridPoint startGrid=worldToGrid(start);
    const GridPoint targetGrid=worldToGrid(targetPosition);
    if(startGrid.x==targetGrid.x)
    {
        const int begin=std::min(startGrid.y,targetGrid.y)+1;
        const int end=std::max(startGrid.y,targetGrid.y);
        for(int y=begin;y<end;++y)
        {
            const Tile* tile=map.tileAt(startGrid.x,y);
            if(tile==nullptr || tile->blocksBullet()) return false;
        }
        return true;
    }
    const int begin=std::min(startGrid.x,targetGrid.x)+1;
    const int end=std::max(startGrid.x,targetGrid.x);
    for(int x=begin;x<end;++x)
    {
        const Tile* tile=map.tileAt(x,startGrid.y);
        if(tile==nullptr || tile->blocksBullet()) return false;
    }
    return true;
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

    for(const ShotCell& shotCell:shotCells)
    {
        if(shotCell.weight<minimumWeight)
            continue;

        std::vector<GridPoint> path=findPath(self,map,map.tileCenter(shotCell.point.x,shotCell.point.y));
        if(path.empty())
            continue;

        if(bestPath.empty() || path.size()<bestPath.size())
            bestPath=path;
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
        return false;
    }
    const float deltaX=currentPosition.x-lastPosition_.x;
    const float deltaY=currentPosition.y-lastPosition_.y;
    const float distanceSquared=deltaX*deltaX+deltaY*deltaY;
    lastPosition_=currentPosition;
    if(!currentCommand_.move || distanceSquared>PositionEpsilonSquared)
    {
        stuckTimer_=0.f;
        return false;
    }
    stuckTimer_+=deltaTime;
    if(stuckTimer_<StuckTimeout) return false;
    stuckTimer_=0.f;
    return true;
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
    if(probability<=0.f) return false;
    if(probability>=1.f) return true;
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
        return false;
    }

    const float deltaX=currentPosition.x-lastPosition_.x;
    const float deltaY=currentPosition.y-lastPosition_.y;
    const float distanceSquared=deltaX*deltaX+deltaY*deltaY;
    lastPosition_=currentPosition;

    if(!currentCommand_.move || distanceSquared>PositionEpsilonSquared)
    {
        stuckTimer_=0.f;
        return false;
    }

    stuckTimer_+=deltaTime;

    if(stuckTimer_<StuckTimeout)
        return false;

    stuckTimer_=0.f;
    return true;
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
    if(probability<=0.f) return false;
    if(probability>=1.f) return true;
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
