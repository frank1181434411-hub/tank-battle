#pragma once
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <optional>
#include <queue>
#include <string>
#include "network/network_types.hpp"
#include "network/network_message.hpp"

class NetworkSession {
public:
    NetworkSession();
    ~NetworkSession();
    NetworkSession(const NetworkSession&) = delete;
    NetworkSession& operator=(const NetworkSession&) = delete;

    bool startHost(unsigned short port);
    bool connectToHost(const std::string& address, unsigned short port);
    void disconnect();
    void update();

    NetworkMode mode() const noexcept { return mode_; }
    NetworkState state() const noexcept { return state_; }
    bool isConnected() const noexcept;
    bool isHost() const noexcept { return mode_ == NetworkMode::Host; }
    bool isClient() const noexcept { return mode_ == NetworkMode::Client; }
    bool isOffline() const noexcept { return mode_ == NetworkMode::Offline; }

    bool sendInput(const PlayerInputState& input);
    bool sendSnapshot(const GameSnapshot& snapshot);
    bool sendTileChanged(int x, int y, int tileType);
    void sendRawPacket(sf::TcpSocket& sock, const sf::Packet& pkt);

    bool pollRemoteInput(PlayerInputState& input);
    bool pollSnapshot(GameSnapshot& snapshot);
    bool pollTileChanged(int& x, int& y, int& tileType);
    bool pollWelcome();

    std::string getRemoteAddress() const;

private:
    void setState(NetworkState);
    void updateHost();
    void updateClient();
    void receiveMessages(sf::TcpSocket& socket);
    void handleMessage(sf::Packet& packet);

    NetworkMode mode_;
    NetworkState state_;
    sf::TcpListener listener_;
    sf::TcpSocket socket_;
    sf::TcpSocket clientSocket_;
    std::queue<sf::Packet> receiveQueue_;

    std::optional<PlayerInputState> pendingRemoteInput_;
    std::optional<GameSnapshot> pendingSnapshot_;
    std::optional<std::pair<int, int>> pendingTileChange_;
    std::optional<int> pendingTileType_;
    bool pendingWelcome_ = false;
    bool isHostConnected_ = false;

    sf::Clock lastReceiveTime_;
    sf::Clock pingClock_;
};