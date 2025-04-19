#include "network/tcp_server.hpp"
#include <iostream>

namespace network {
// TCPConnection implementation
TCPConnection::TCPConnection(boost::asio::ip::tcp::socket socket)
    : socket_(std::move(socket)),
      message_handler_([](uint8_t const*, std::size_t, std::shared_ptr<TCPConnection>) {
      }),
      disconnect_handler_([](std::shared_ptr<TCPConnection>) {
      }) {
}

void TCPConnection::start() {
    start_read();
}

void TCPConnection::send_data(uint8_t const* data, std::size_t length) {
    if (!socket_.is_open()) {
        return;
    }
    auto self = shared_from_this();
    boost::asio::async_write(socket_,
        boost::asio::buffer(data, length),
        [this](boost::system::error_code const& error, auto) {
            if (error) {
                log_error("Send error: " + error.message());
                if (error == boost::asio::error::connection_reset ||
                    error == boost::asio::error::broken_pipe) {
                    close();
                }
            }
        });
}

void TCPConnection::close() {
    if (socket_.is_open()) {
        boost::system::error_code ec;
        std::ignore = socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        std::ignore = socket_.close(ec);
        auto self = shared_from_this();
        disconnect_handler_(self);
    }
}

std::string TCPConnection::get_endpoint_string() const {
    try {
        auto endpoint = socket_.remote_endpoint();
        return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    } catch (std::exception const&) {
        return "unknown";
    }
}

boost::asio::ip::tcp::endpoint TCPConnection::get_endpoint() const {
    return socket_.remote_endpoint();
}

void TCPConnection::set_message_handler(MessageHandler handler) {
    message_handler_ = std::move(handler);
}

void TCPConnection::set_disconnect_handler(DisconnectHandler handler) {
    disconnect_handler_ = std::move(handler);
}

void TCPConnection::start_read() {
    auto self = shared_from_this();
    socket_.async_read_some(
        boost::asio::buffer(recv_buffer_),
        [this](boost::system::error_code const& error, std::size_t bytes_transferred) {
            this->handle_read(error, bytes_transferred);
        });
}

void TCPConnection::handle_read(boost::system::error_code const& error,
    std::size_t bytes_transferred) {
    if (!error) {
        auto self = shared_from_this();

        // Call the message handler with binary data for flatbuffer
        message_handler_(recv_buffer_.data(), bytes_transferred, self);

        // Continue reading
        start_read();
    } else if (error == boost::asio::error::eof ||
               error == boost::asio::error::connection_reset) {
        log_info("Client disconnected: " + get_endpoint_string());
        close();
    } else {
        log_error("Read error: " + error.message());
        close();
    }
}

// TCPServer implementation
TCPServer::TCPServer(boost::asio::io_context& io_context, int port)
    : io_context_(io_context),
      acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      running_(false),
      connection_handler_([](std::shared_ptr<TCPConnection>) {
      }) {
    log_info("TCP server initialized on port " + std::to_string(port));
}

void TCPServer::start() {
    if (!running_) {
        running_ = true;
        start_accept();
        log_info("TCP server started");
    }
}

void TCPServer::stop() {
    if (running_) {
        running_ = false;
        acceptor_.close();

        // Close all connections
        for (auto& connection : connections_) {
            connection->close();
        }
        connections_.clear();

        log_info("TCP server stopped");
    }
}

void TCPServer::broadcast(uint8_t const* data, std::size_t length) const {
    for (auto& connection : connections_) {
        connection->send_data(data, length);
    }
}

std::size_t TCPServer::connection_count() const {
    return connections_.size();
}

void TCPServer::set_connection_handler(ConnectionHandler handler) {
    connection_handler_ = std::move(handler);
}

void TCPServer::start_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code const& error, boost::asio::ip::tcp::socket socket) {
            auto connection = std::make_shared<TCPConnection>(std::move(socket));
            this->handle_accept(connection, error);
        });
}

void TCPServer::handle_accept(std::shared_ptr<TCPConnection> connection,
    boost::system::error_code const& error) {
    if (!error) {
        log_info("New TCP connection from: " + connection->get_endpoint_string());

        // Set disconnect handler
        connection->set_disconnect_handler(
            [this](std::shared_ptr<TCPConnection> conn) {
                this->handle_client_disconnect(std::move(conn));
            });

        // Add to connections set
        connections_.insert(connection);

        // Notify about new connection
        connection_handler_(connection);

        // Start reading data
        connection->start();
    } else {
        log_error("Accept error: " + error.message());
    }

    // Continue accepting if still running
    if (running_) {
        start_accept();
    }
}

void TCPServer::handle_client_disconnect(std::shared_ptr<TCPConnection> connection) {
    connections_.erase(connection);
}
} // namespace network
