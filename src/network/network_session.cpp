#include "network/network_session.hpp"
#include <iostream>

NetworkSession::NetworkSession()
    : mode_(NetworkMode::Offline),
    state_(NetworkState::Disconnected) {
    socket_.setBlocking(false);
    clientSocket_.setBlocking(false);
    listener_.setBlocking(false);
}

NetworkSession::~NetworkSession() {
    disconnect();
}

void NetworkSession::sendRawPacket(sf::TcpSocket& sock, const sf::Packet& pkt) {
    sf::Packet packet = pkt;
    sock.setBlocking(true);
    const sf::Socket::Status status = sock.send(packet);
    sock.setBlocking(false);
    if (status != sf::Socket::Status::Done) {
        std::cerr << "[Net] Send failed" << std::endl;
    }
}

bool NetworkSession::startHost(unsigned short port) {
    if (mode_ != NetworkMode::Offline) disconnect();
    if (listener_.listen(port) != sf::Socket::Status::Done) {
        std::cerr << "[Net] Listen failed" << std::endl;
        setState(NetworkState::Failed);
        return false;
    }
    mode_ = NetworkMode::Host;
    setState(NetworkState::Hosting);
    std::cout << "[Net] Host start port:" << port << std::endl;
    return true;
}

bool NetworkSession::connectToHost(const std::string& address, unsigned short port) {
    if (mode_ != NetworkMode::Offline) disconnect();
    setState(NetworkState::Connecting);
    mode_ = NetworkMode::Client;

    const auto addresses = sf::Dns::resolve(address);
    if (!addresses.has_value() || addresses->empty()) {
        std::cerr << "[Net] Invalid address!" << std::endl;
        mode_ = NetworkMode::Offline;
        setState(NetworkState::Failed);
        return false;
    }
    sf::Socket::Status status = socket_.connect(addresses->front(), port, sf::seconds(5.f));

    if (status != sf::Socket::Status::Done) {
        std::cerr << "[Net] Connect failed!" << std::endl;
        mode_ = NetworkMode::Offline;
        setState(NetworkState::Failed);
        return false;
    }

    socket_.setBlocking(false);
    sf::Packet helloPkt = NetworkMessage::makeHello();
    sendRawPacket(socket_, helloPkt);

    setState(NetworkState::Connected);
    std::cout << "[Net] Connected!" << std::endl;
    return true;
}

void NetworkSession::disconnect() {
    listener_.close();
    socket_.disconnect();
    clientSocket_.disconnect();
    mode_ = NetworkMode::Offline;
    setState(NetworkState::Disconnected);
    isHostConnected_ = false;
    pendingRemoteInput_.reset();
    pendingSnapshot_.reset();
    pendingTileChange_.reset();
    pendingTileType_.reset();
    pendingWelcome_ = false;
    std::cout << "[Net] Disconnected" << std::endl;
}

void NetworkSession::update() {
    if (mode_ == NetworkMode::Host) updateHost();
    else if (mode_ == NetworkMode::Client) updateClient();
}

void NetworkSession::updateHost() {
    if (state_ == NetworkState::Hosting) {
        if (listener_.accept(clientSocket_) == sf::Socket::Status::Done) {
            clientSocket_.setBlocking(false);
            isHostConnected_ = true;
            setState(NetworkState::Connected);
            std::cout << "[Net] Client connected!" << std::endl;
        }
    }

    if (isHostConnected_ && state_ == NetworkState::Connected) {
        sf::Packet packet;
        while (clientSocket_.receive(packet) == sf::Socket::Status::Done) {
            std::cout << "[Net] Host received something!" << std::endl;
            handleMessage(packet);
            packet.clear();
        }
    }
}

void NetworkSession::updateClient() {
    if (state_ == NetworkState::Connected) {
        sf::Packet packet;
        while (socket_.receive(packet) == sf::Socket::Status::Done) {
            handleMessage(packet);
            packet.clear();
        }
    }
}

void NetworkSession::handleMessage(sf::Packet& packet) {
    NetworkMessage::MessageType type;
    if (!NetworkMessage::readType(packet, type)) return;

    switch (type) {
    case NetworkMessage::MessageType::Hello:
        std::cout << "[Net] Received Hello!" << std::endl;
        pendingWelcome_ = true;
        if (mode_ == NetworkMode::Host && isHostConnected_) {
            sf::Packet welcomePkt = NetworkMessage::makeWelcome();
            std::cout << "[Net] Sending Welcome..." << std::endl;
            sendRawPacket(clientSocket_, welcomePkt);
        }
        break;

    case NetworkMessage::MessageType::Welcome:
        std::cout << "[Net] Received Welcome!" << std::endl;
        pendingWelcome_ = true;
        break;

    case NetworkMessage::MessageType::PlayerInput: {
        PlayerInputState input;
        if (NetworkMessage::readPlayerInput(packet, input))
            pendingRemoteInput_ = input;
        break;
    }

    case NetworkMessage::MessageType::GameSnapshot: {
        GameSnapshot snap;
        if (NetworkMessage::readGameSnapshot(packet, snap))
            pendingSnapshot_ = snap;
        break;
    }

    case NetworkMessage::MessageType::TileChanged: {
        int x, y, t;
        if (NetworkMessage::readTileChanged(packet, x, y, t)) {
            pendingTileChange_ = { x, y };
            pendingTileType_ = t;
        }
        break;
    }

    case NetworkMessage::MessageType::Disconnect:
        disconnect();
        break;

    default:
        break;
    }
}

bool NetworkSession::sendInput(const PlayerInputState& input) {
    if (!isConnected() || mode_ != NetworkMode::Client) return false;
    sf::Packet pkt = NetworkMessage::makePlayerInput(input);
    sendRawPacket(socket_, pkt);
    return true;
}

bool NetworkSession::sendSnapshot(const GameSnapshot& snapshot) {
    if (!isConnected() || mode_ != NetworkMode::Host) return false;
    sf::Packet pkt = NetworkMessage::makeGameSnapshot(snapshot);
    sendRawPacket(clientSocket_, pkt);
    return true;
}

bool NetworkSession::sendTileChanged(int x, int y, int tileType) {
    if (!isConnected() || mode_ != NetworkMode::Host) return false;
    sf::Packet pkt = NetworkMessage::makeTileChanged(x, y, tileType);
    sendRawPacket(clientSocket_, pkt);
    return true;
}

bool NetworkSession::pollRemoteInput(PlayerInputState& input) {
    if (pendingRemoteInput_.has_value()) {
        input = pendingRemoteInput_.value();
        pendingRemoteInput_.reset();
        return true;
    }
    return false;
}

bool NetworkSession::pollSnapshot(GameSnapshot& snapshot) {
    if (pendingSnapshot_.has_value()) {
        snapshot = pendingSnapshot_.value();
        pendingSnapshot_.reset();
        return true;
    }
    return false;
}

bool NetworkSession::pollTileChanged(int& x, int& y, int& tileType) {
    if (pendingTileChange_.has_value() && pendingTileType_.has_value()) {
        x = pendingTileChange_->first;
        y = pendingTileChange_->second;
        tileType = pendingTileType_.value();
        pendingTileChange_.reset();
        pendingTileType_.reset();
        return true;
    }
    return false;
}

bool NetworkSession::pollWelcome() {
    if (pendingWelcome_) {
        pendingWelcome_ = false;
        return true;
    }
    return false;
}

bool NetworkSession::isConnected() const noexcept {
    return state_ == NetworkState::Connected;
}

void NetworkSession::setState(NetworkState state) {
    state_ = state;
}

std::string NetworkSession::getRemoteAddress() const {
    if (mode_ == NetworkMode::Client) {
        auto addr = socket_.getRemoteAddress();
        return addr.has_value() ? addr.value().toString() : "";
    }
    else if (mode_ == NetworkMode::Host && isHostConnected_) {
        auto addr = clientSocket_.getRemoteAddress();
        return addr.has_value() ? addr.value().toString() : "";
    }
    return "";
}
