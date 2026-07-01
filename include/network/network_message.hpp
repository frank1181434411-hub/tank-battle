#pragma once
#include "network/network_types.hpp"
#include <SFML/Network.hpp>

namespace NetworkMessage
{
    enum class MessageType : uint8_t
    {
        Hello,
        Welcome,
        PlayerInput,
        GameSnapshot,
        TileChanged,
        Disconnect,
        Ping,
        Pong
    };

    sf::Packet makeHello();
    sf::Packet makeWelcome();
    sf::Packet makePlayerInput(const PlayerInputState& input);
    sf::Packet makeGameSnapshot(const GameSnapshot& snap);
    sf::Packet makeTileChanged(int x, int y, int tileType);
    sf::Packet makeDisconnect();
    sf::Packet makePing();
    sf::Packet makePong();

    bool readType(sf::Packet& pkt, MessageType& type);
    bool readPlayerInput(sf::Packet& pkt, PlayerInputState& out);
    bool readTileChanged(sf::Packet& pkt, int& x, int& y, int& t);
    bool readGameSnapshot(sf::Packet& pkt, GameSnapshot& out);

    void writeTankSnapshot(sf::Packet& pkt, const TankSnapshot& t);
    bool readTankSnapshot(sf::Packet& pkt, TankSnapshot& t);

    void writeBulletSnapshot(sf::Packet& pkt, const BulletSnapshot& b);
    bool readBulletSnapshot(sf::Packet& pkt, BulletSnapshot& b);

    void writeItemSnapshot(sf::Packet& pkt, const ItemSnapshot& i);
    bool readItemSnapshot(sf::Packet& pkt, ItemSnapshot& i);
}