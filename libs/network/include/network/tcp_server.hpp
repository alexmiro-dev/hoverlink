#pragma once

#include <memory>
#include <string>
#include <functional>
#include <set>
#include <boost/asio.hpp>
#include "network/common.hpp"

namespace network {
class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
public:
    using MessageHandler = std::function<void(uint8_t const*, std::size_t,
                                              std::shared_ptr<TCPConnection>)>;
    using DisconnectHandler = std::function<void(std::shared_ptr<TCPConnection>)>;

    explicit TCPConnection(boost::asio::ip::tcp::socket socket);

    // Start reading data from the connection
    void start();

    // Send binary data (flatbuffers)
    void send_data(uint8_t const* data, std::size_t length);

    // Close the connection
    void close();

    // Get endpoint information
    std::string get_endpoint_string() const;
    boost::asio::ip::tcp::endpoint get_endpoint() const;

    // Set handlers
    void set_message_handler(MessageHandler handler);
    void set_disconnect_handler(DisconnectHandler handler);

private:
    void start_read();
    void handle_read(boost::system::error_code const& error, std::size_t bytes_transferred);

    boost::asio::ip::tcp::socket socket_;
    std::array<uint8_t, MAX_BUFFER_SIZE> recv_buffer_;
    MessageHandler message_handler_;
    DisconnectHandler disconnect_handler_;
};

class TCPServer {
public:
    using ConnectionHandler = std::function<void(std::shared_ptr<TCPConnection>)>;

    explicit TCPServer(boost::asio::io_context& io_context,
                       int port = DEFAULT_TCP_PORT);

    // Start accepting connections
    void start();

    // Stop the server
    void stop();

    // Broadcast data to all clients
    void broadcast(uint8_t const* data, std::size_t length) const;

    // Get number of connected clients
    [[nodiscard]] std::size_t connection_count() const;

    // Set handlers
    void set_connection_handler(ConnectionHandler handler);

private:
    void start_accept();
    void handle_accept(std::shared_ptr<TCPConnection> connection,
                       boost::system::error_code const& error);
    void handle_client_disconnect(std::shared_ptr<TCPConnection> connection);

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    bool running_;
    std::set<std::shared_ptr<TCPConnection>> connections_;
    ConnectionHandler connection_handler_;
};
} // namespace network
