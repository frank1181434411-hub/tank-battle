#include "network/network_message.hpp"
#include <iostream>

namespace NetworkMessage
{
    sf::Packet makeHello()
    {
        sf::Packet p;
        p << static_cast<uint8_t>(MessageType::Hello);
        p << uint8_t(1);
        return p;
    }
    sf::Packet makeWelcome()
    {
        sf::Packet p;
        p << static_cast<uint8_t>(MessageType::Welcome);
        p << uint8_t(1);
        return p;
    }
    sf::Packet makePlayerInput(const PlayerInputState& input)
    {
        sf::Packet p;
        p << static_cast<uint8_t>(MessageType::PlayerInput);
        p << input.moveUp << input.moveDown << input.moveLeft << input.moveRight << input.fire;
        return p;
    }
    sf::Packet makeGameSnapshot(const GameSnapshot& snap)
    {
        sf::Packet p;
        p << static_cast<uint8_t>(MessageType::GameSnapshot);
        p << snap.status;
        writeTankSnapshot(p, snap.playerOne);
        writeTankSnapshot(p, snap.playerTwo);
        uint32_t n = static_cast<uint32_t>(snap.enemies.size());
        p << n;
        for (auto& t : snap.enemies) writeTankSnapshot(p, t);
        n = static_cast<uint32_t>(snap.bullets.size());
        p << n;
        for (auto& b : snap.bullets) writeBulletSnapshot(p, b);
        n = static_cast<uint32_t>(snap.items.size());
        p << n;
        for (auto& i : snap.items) writeItemSnapshot(p, i);
        p << snap.baseHp << snap.baseAlive;
        n = static_cast<uint32_t>(snap.destroyedBricks.size());
        p << n;
        for (auto& pr : snap.destroyedBricks) p << pr.first << pr.second;
        return p;
    }
    sf::Packet makeTileChanged(int x, int y, int tileType)
    {
        sf::Packet p;
        p << static_cast<uint8_t>(MessageType::TileChanged);
        p << x << y << tileType;
        return p;
    }
    sf::Packet makeDisconnect()
    {
        sf::Packet p;
        p << static_cast<uint8_t>(MessageType::Disconnect);
        return p;
    }
    sf::Packet makePing()
    {
        sf::Packet p;
        p << static_cast<uint8_t>(MessageType::Ping);
        p << int32_t(0);
        return p;
    }
    sf::Packet makePong()
    {
        sf::Packet p;
        p << static_cast<uint8_t>(MessageType::Pong);
        return p;
    }

    bool readType(sf::Packet& pkt, MessageType& type)
    {
        uint8_t t;
        if (!(pkt >> t)) return false;
        type = static_cast<MessageType>(t);
        return true;
    }
    bool readPlayerInput(sf::Packet& pkt, PlayerInputState& out)
    {
        if (!(pkt >> out.moveUp)) return false;
        if (!(pkt >> out.moveDown)) return false;
        if (!(pkt >> out.moveLeft)) return false;
        if (!(pkt >> out.moveRight)) return false;
        if (!(pkt >> out.fire)) return false;
        return true;
    }
    bool readTileChanged(sf::Packet& pkt, int& x, int& y, int& t)
    {
        if (!(pkt >> x)) return false;
        if (!(pkt >> y)) return false;
        if (!(pkt >> t)) return false;
        return true;
    }
    void writeTankSnapshot(sf::Packet& pkt, const TankSnapshot& t)
    {
        pkt << t.id << t.x << t.y << t.direction << t.hp << t.alive;
    }
    bool readTankSnapshot(sf::Packet& pkt, TankSnapshot& t)
    {
        if (!(pkt >> t.id)) return false;
        if (!(pkt >> t.x)) return false;
        if (!(pkt >> t.y)) return false;
        if (!(pkt >> t.direction)) return false;
        if (!(pkt >> t.hp)) return false;
        if (!(pkt >> t.alive)) return false;
        return true;
    }
    void writeBulletSnapshot(sf::Packet& pkt, const BulletSnapshot& b)
    {
        pkt << b.x << b.y << b.faction << b.damage << b.ownerId << b.alive << b.dirX << b.dirY;
    }
    bool readBulletSnapshot(sf::Packet& pkt, BulletSnapshot& b)
    {
        if (!(pkt >> b.x)) return false;
        if (!(pkt >> b.y)) return false;
        if (!(pkt >> b.faction)) return false;
        if (!(pkt >> b.damage)) return false;
        if (!(pkt >> b.ownerId)) return false;
        if (!(pkt >> b.alive)) return false;
        if (!(pkt >> b.dirX)) return false;
        if (!(pkt >> b.dirY)) return false;
        return true;
    }
    void writeItemSnapshot(sf::Packet& pkt, const ItemSnapshot& i)
    {
        pkt << i.x << i.y << i.type << i.alive;
    }
    bool readItemSnapshot(sf::Packet& pkt, ItemSnapshot& i)
    {
        if (!(pkt >> i.x)) return false;
        if (!(pkt >> i.y)) return false;
        if (!(pkt >> i.type)) return false;
        if (!(pkt >> i.alive)) return false;
        return true;
    }
    bool readGameSnapshot(sf::Packet& pkt, GameSnapshot& snap)
    {
        if (!(pkt >> snap.status)) return false;
        if (!readTankSnapshot(pkt, snap.playerOne)) return false;
        if (!readTankSnapshot(pkt, snap.playerTwo)) return false;
        uint32_t n;
        if (!(pkt >> n)) return false;
        snap.enemies.clear();
        for (uint32_t i = 0; i < n; i++)
        {
            TankSnapshot t;
            if (!readTankSnapshot(pkt, t)) return false;
            snap.enemies.push_back(t);
        }
        if (!(pkt >> n)) return false;
        snap.bullets.clear();
        for (uint32_t i = 0; i < n; i++)
        {
            BulletSnapshot b;
            if (!readBulletSnapshot(pkt, b)) return false;
            snap.bullets.push_back(b);
        }
        if (!(pkt >> n)) return false;
        snap.items.clear();
        for (uint32_t i = 0; i < n; i++)
        {
            ItemSnapshot it;
            if (!readItemSnapshot(pkt, it)) return false;
            snap.items.push_back(it);
        }
        if (!(pkt >> snap.baseHp)) return false;
        if (!(pkt >> snap.baseAlive)) return false;
        if (!(pkt >> n)) return false;
        snap.destroyedBricks.clear();
        for (uint32_t i = 0; i < n; i++)
        {
            int x, y;
            if (!(pkt >> x >> y)) return false;
            snap.destroyedBricks.emplace_back(x, y);
        }
        return true;
    }
}