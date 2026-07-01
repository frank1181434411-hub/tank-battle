#include "core/game.hpp"
#include "world/collision.hpp"

#include <cmath>
#include <cstddef>
#include <optional>

namespace {
Direction directionFromVelocity(sf::Vector2f velocity) noexcept
{
    if(std::abs(velocity.x)>std::abs(velocity.y))
    {
        if(velocity.x<0.f)
            return Direction::Left;
        return Direction::Right;
    }
    if(velocity.y>0.f)
        return Direction::Down;
    return Direction::Up;
}

float speedFromVelocity(sf::Vector2f velocity) noexcept
{
    const float speed=std::sqrt(velocity.x*velocity.x+velocity.y*velocity.y);
    if(speed>0.f)
        return speed;
    return prototype::PlayerBulletSpeed;
}

void addAliveEnemyBlocks(std::vector<sf::FloatRect>& blocks,const std::vector<EnemyTank>& enemies)
{
    for(const auto& enemy:enemies)
    {
        if(enemy.isAlive())
            blocks.push_back(enemy.getBounds());
    }
}

void updatePlayerByInput(PlayerTank& player,float deltaTime,std::vector<Bullet>& bullets,const Map& map,const std::vector<sf::FloatRect>& blocks,const PlayerInputState& input)
{
    player.updateWithInput(deltaTime,bullets,map,blocks,input.moveUp,input.moveDown,input.moveLeft,input.moveRight,input.fire);
}
}

Game::Game()
    : window_(sf::VideoMode({static_cast<unsigned int>(prototype::WindowWidth),
                             static_cast<unsigned int>(prototype::WindowHeight)}),
              "Retro Tank Battle"),
      map_(),
      networkSession_(),
      playerOne_(),
      playerTwo_(),
      base_(),
      enemies_(),
      bullets_(),
      items_(),
      localInput_(),
      remoteInput_(),
      status_(GameStatus::Running)
{
    window_.setFramerateLimit(60);

    const bool mapLoaded=map_.loadFromFile("assets/maps/map_test.txt");

    sf::Vector2f playerPosition(400.f,500.f);
    if(mapLoaded && map_.hasPlayerSpawn()) playerPosition=map_.playerSpawn();
    playerOne_.emplace(playerPosition.x,playerPosition.y);

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

void Game::startHost(unsigned short port)
{
    networkSession_.startHost(port);
}

void Game::joinHost(const std::string& address,unsigned short port)
{
    networkSession_.connectToHost(address,port);
}

void Game::leaveNetworkGame()
{
    networkSession_.disconnect();
}

void Game::run() {
    sf::Clock clock;
    while (window_.isOpen()) {
        const float deltaTime = clock.restart().asSeconds();
        processEvents();
        networkSession_.update();
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

void Game::update(float deltaTime)
{
    collectLocalInput(localInput_);

    if(networkSession_.isOffline())
        updateOffline(deltaTime);
    else if(networkSession_.isHost())
        updateHost(deltaTime);
    else if(networkSession_.isClient())
        updateClient(deltaTime);
}

void Game::updateOffline(float deltaTime)
{
    if (status_ != GameStatus::Running || !playerOne_ || !base_) {
        return;
    }

    std::vector<sf::FloatRect> playerTankBlocks;
    if(base_->isAlive())
        playerTankBlocks.push_back(base_->getBounds());
    addAliveEnemyBlocks(playerTankBlocks,enemies_);
    playerOne_->update(deltaTime,bullets_,map_,playerTankBlocks);

    for (auto& enemy : enemies_) {
        std::vector<sf::FloatRect> enemyTankBlocks;
        if(playerOne_->isAlive())
            enemyTankBlocks.push_back(playerOne_->getBounds());
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
        if (item.isAlive() && playerOne_->getBounds().findIntersection(item.getBounds()).has_value()) {
            playerOne_->collectItem(item);
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
            if (playerOne_->isAlive() && bullet->getBounds().findIntersection(playerOne_->getBounds()).has_value()) {
                playerOne_->takeDamage(bullet->getDamage());
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

void Game::updateHost(float deltaTime)
{
    if(status_!=GameStatus::Running || !playerOne_ || !base_)
        return;

    PlayerInputState remoteInput;
    if(networkSession_.pollRemoteInput(remoteInput))
        remoteInput_=remoteInput;

    if(networkSession_.isConnected() && !hasTwoPlayers_)
    {
        sf::Vector2f position=playerOne_->getPosition();
        position.x-=60.f;
        playerTwo_.emplace(position.x,position.y);
        playerTwo_->setColor(sf::Color::Cyan);
        hasTwoPlayers_=true;
    }

    std::vector<sf::FloatRect> playerOneBlocks;
    if(base_->isAlive())
        playerOneBlocks.push_back(base_->getBounds());
    addAliveEnemyBlocks(playerOneBlocks,enemies_);
    if(playerTwo_ && playerTwo_->isAlive())
        playerOneBlocks.push_back(playerTwo_->getBounds());
    updatePlayerByInput(*playerOne_,deltaTime,bullets_,map_,playerOneBlocks,localInput_);

    if(playerTwo_)
    {
        std::vector<sf::FloatRect> playerTwoBlocks;
        if(base_->isAlive())
            playerTwoBlocks.push_back(base_->getBounds());
        addAliveEnemyBlocks(playerTwoBlocks,enemies_);
        if(playerOne_->isAlive())
            playerTwoBlocks.push_back(playerOne_->getBounds());
        updatePlayerByInput(*playerTwo_,deltaTime,bullets_,map_,playerTwoBlocks,remoteInput_);
    }

    for(auto& enemy:enemies_)
    {
        std::vector<sf::FloatRect> enemyTankBlocks;
        if(playerOne_->isAlive())
            enemyTankBlocks.push_back(playerOne_->getBounds());
        if(playerTwo_ && playerTwo_->isAlive())
            enemyTankBlocks.push_back(playerTwo_->getBounds());
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

    for(auto& item:items_)
    {
        if(!item.isAlive())
            continue;
        bool picked=false;
        if(playerOne_->isAlive() && playerOne_->getBounds().findIntersection(item.getBounds()).has_value())
        {
            playerOne_->collectItem(item);
            picked=true;
        }
        if(!picked && playerTwo_ && playerTwo_->isAlive() && playerTwo_->getBounds().findIntersection(item.getBounds()).has_value())
            playerTwo_->collectItem(item);
    }

    for(auto bullet=bullets_.begin();bullet!=bullets_.end();)
    {
        bullet->update(deltaTime);
        bool hit=false;

        int hitTileX=0;
        int hitTileY=0;
        if(Collision::findBulletHitTile(map_,bullet->getBounds(),hitTileX,hitTileY))
        {
            const Tile* tile=map_.tileAt(hitTileX,hitTileY);
            if(tile!=nullptr && tile->type()==TileType::Brick)
            {
                map_.setTile(hitTileX,hitTileY,TileType::Empty);
                networkSession_.sendTileChanged(hitTileX,hitTileY,static_cast<int>(TileType::Empty));
            }
            hit=true;
        }
        else if(Collision::hitsMapBounds(map_,bullet->getBounds()))
            hit=true;

        if(!hit && bullet->getFaction()==Faction::Player)
        {
            for(auto& enemy:enemies_)
            {
                if(enemy.isAlive() && bullet->getBounds().findIntersection(enemy.getBounds()).has_value())
                {
                    enemy.takeDamage(bullet->getDamage());
                    hit=true;
                    break;
                }
            }
        }
        else if(!hit && bullet->getFaction()==Faction::Enemy)
        {
            if(playerOne_->isAlive() && bullet->getBounds().findIntersection(playerOne_->getBounds()).has_value())
            {
                playerOne_->takeDamage(bullet->getDamage());
                hit=true;
            }
            else if(playerTwo_ && playerTwo_->isAlive() && bullet->getBounds().findIntersection(playerTwo_->getBounds()).has_value())
            {
                playerTwo_->takeDamage(bullet->getDamage());
                hit=true;
            }
            else if(base_->isAlive() && bullet->getBounds().findIntersection(base_->getBounds()).has_value())
            {
                base_->takeDamage(bullet->getDamage());
                hit=true;
            }
        }

        if(hit || !bullet->isAlive())
            bullet=bullets_.erase(bullet);
        else
            ++bullet;
    }

    for(auto enemy=enemies_.begin();enemy!=enemies_.end();)
    {
        if(!enemy->isAlive())
            enemy=enemies_.erase(enemy);
        else
            ++enemy;
    }

    for(auto item=items_.begin();item!=items_.end();)
    {
        if(!item->isAlive())
            item=items_.erase(item);
        else
            ++item;
    }

    updateStatus();
    networkSession_.sendSnapshot(createSnapshot());
}

void Game::updateClient(float)
{
    GameSnapshot snapshot;
    if(networkSession_.pollSnapshot(snapshot))
        applySnapshot(snapshot);

    int tileX=0;
    int tileY=0;
    int tileType=0;
    if(networkSession_.pollTileChanged(tileX,tileY,tileType))
        map_.setTile(tileX,tileY,static_cast<TileType>(tileType));

    networkSession_.sendInput(localInput_);
}

void Game::collectLocalInput(PlayerInputState& input)
{
    input.moveUp=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
    input.moveDown=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    input.moveLeft=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    input.moveRight=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    input.fire=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
}

void Game::applySnapshot(const GameSnapshot& snapshot)
{
    status_=static_cast<GameStatus>(snapshot.status);

    if(!playerOne_)
        playerOne_.emplace(snapshot.playerOne.x,snapshot.playerOne.y);
    playerOne_->setPosition({snapshot.playerOne.x,snapshot.playerOne.y});
    playerOne_->setDirection(static_cast<Direction>(snapshot.playerOne.direction));
    playerOne_->setHp(snapshot.playerOne.hp);

    if(snapshot.playerTwo.alive && !playerTwo_)
    {
        playerTwo_.emplace(snapshot.playerTwo.x,snapshot.playerTwo.y);
        playerTwo_->setColor(sf::Color::Cyan);
        hasTwoPlayers_=true;
    }
    if(playerTwo_)
    {
        playerTwo_->setPosition({snapshot.playerTwo.x,snapshot.playerTwo.y});
        playerTwo_->setDirection(static_cast<Direction>(snapshot.playerTwo.direction));
        playerTwo_->setHp(snapshot.playerTwo.hp);
    }

    enemies_.clear();
    for(const auto& enemySnapshot:snapshot.enemies)
    {
        EnemyTank enemy(enemySnapshot.x,enemySnapshot.y,EnemyType::Light);
        enemy.setDirection(static_cast<Direction>(enemySnapshot.direction));
        enemy.setHp(enemySnapshot.hp);
        enemies_.push_back(enemy);
    }

    bullets_.clear();
    for(const auto& bulletSnapshot:snapshot.bullets)
    {
        if(!bulletSnapshot.alive)
            continue;
        const sf::Vector2f velocity{bulletSnapshot.dirX,bulletSnapshot.dirY};
        const Direction direction=directionFromVelocity(velocity);
        const float speed=speedFromVelocity(velocity);
        bullets_.emplace_back(sf::Vector2f{bulletSnapshot.x,bulletSnapshot.y},direction,static_cast<Faction>(bulletSnapshot.faction),bulletSnapshot.damage,speed,bulletSnapshot.ownerId);
    }

    items_.clear();
    for(const auto& itemSnapshot:snapshot.items)
    {
        if(itemSnapshot.alive)
            items_.emplace_back(sf::Vector2f{itemSnapshot.x,itemSnapshot.y},static_cast<ItemType>(itemSnapshot.type));
    }

    if(base_)
        base_->setHp(snapshot.baseHp);
}

GameSnapshot Game::createSnapshot() const
{
    GameSnapshot snapshot;
    snapshot.status=static_cast<int>(status_);

    if(playerOne_)
    {
        snapshot.playerOne.id=playerOne_->getId();
        snapshot.playerOne.x=playerOne_->getPosition().x;
        snapshot.playerOne.y=playerOne_->getPosition().y;
        snapshot.playerOne.direction=static_cast<int>(playerOne_->getDirection());
        snapshot.playerOne.hp=playerOne_->getHp();
        snapshot.playerOne.alive=playerOne_->isAlive();
    }

    if(playerTwo_)
    {
        snapshot.playerTwo.id=playerTwo_->getId();
        snapshot.playerTwo.x=playerTwo_->getPosition().x;
        snapshot.playerTwo.y=playerTwo_->getPosition().y;
        snapshot.playerTwo.direction=static_cast<int>(playerTwo_->getDirection());
        snapshot.playerTwo.hp=playerTwo_->getHp();
        snapshot.playerTwo.alive=playerTwo_->isAlive();
    }

    for(const auto& enemy:enemies_)
    {
        TankSnapshot enemySnapshot;
        enemySnapshot.id=enemy.getId();
        enemySnapshot.x=enemy.getPosition().x;
        enemySnapshot.y=enemy.getPosition().y;
        enemySnapshot.direction=static_cast<int>(enemy.getDirection());
        enemySnapshot.hp=enemy.getHp();
        enemySnapshot.alive=enemy.isAlive();
        snapshot.enemies.push_back(enemySnapshot);
    }

    for(const auto& bullet:bullets_)
    {
        if(!bullet.isAlive())
            continue;
        const sf::Vector2f velocity=bullet.getVelocity();
        BulletSnapshot bulletSnapshot;
        bulletSnapshot.x=bullet.getPosition().x;
        bulletSnapshot.y=bullet.getPosition().y;
        bulletSnapshot.faction=static_cast<int>(bullet.getFaction());
        bulletSnapshot.damage=bullet.getDamage();
        bulletSnapshot.ownerId=bullet.getOwnerId();
        bulletSnapshot.alive=true;
        bulletSnapshot.dirX=velocity.x;
        bulletSnapshot.dirY=velocity.y;
        snapshot.bullets.push_back(bulletSnapshot);
    }

    for(const auto& item:items_)
    {
        ItemSnapshot itemSnapshot;
        const sf::FloatRect bounds=item.getBounds();
        itemSnapshot.x=bounds.position.x+bounds.size.x/2.f;
        itemSnapshot.y=bounds.position.y+bounds.size.y/2.f;
        itemSnapshot.type=static_cast<int>(item.getType());
        itemSnapshot.alive=item.isAlive();
        snapshot.items.push_back(itemSnapshot);
    }

    if(base_)
    {
        snapshot.baseHp=base_->getHp();
        snapshot.baseAlive=base_->isAlive();
    }

    return snapshot;
}

void Game::render() {
    window_.clear(sf::Color::Black);

    map_.drawLayer(window_,0);
    if(base_) base_->draw(window_);
    for (const auto& item : items_) {
        item.draw(window_);
    }

    if(playerOne_) playerOne_->draw(window_);
    if(playerTwo_) playerTwo_->draw(window_);
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
    if(!playerOne_ || !base_) return;

    bool playersDefeated=!playerOne_->isAlive();
    if(playerTwo_)
        playersDefeated=!playerOne_->isAlive() && !playerTwo_->isAlive();

    if (!base_->isAlive() || playersDefeated) {
        status_ = GameStatus::Defeat;
        window_.setTitle("Retro Tank Battle - Defeat");
    } else if (enemies_.empty()) {
        status_ = GameStatus::Victory;
        window_.setTitle("Retro Tank Battle - Victory");
    }
}
