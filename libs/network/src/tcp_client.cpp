#include "network/tcp_client.hpp"
#include <iostream>

namespace network {

TCPClient::TCPClient(boost::asio::io_context& io_context)
    : io_context_(io_context),
      socket_(std::make_unique<boost::asio::ip::tcp::socket>(io_context)),
      connected_(false),
      message_handler_([](const uint8_t*, std::size_t) {}),
      disconnect_handler_([]() {}) {
    log_info("TCP client initialized");
}

TCPClient::~TCPClient() {
    disconnect();
}

void TCPClient::connect(const std::string& host, int port, ConnectHandler handler) {
    if (connected_) {
        disconnect();
    }
    
    boost::asio::ip::tcp::resolver resolver(io_context_);
    
    try {
        auto endpoints = resolver.resolve(host, std::to_string(port));
        
        boost::asio::async_connect(*socket_, endpoints,
            [this, handler](const boost::system::error_code& error, 
                         const boost::asio::ip::tcp::endpoint& /*endpoint*/) {
                if (!error) {
                    connected_ = true;
                    log_info("Connected to server at " + 
                            socket_->remote_endpoint().address().to_string() + ":" + 
                            std::to_string(socket_->remote_endpoint().port()));
                    
                    start_read();
                    handler(true);
                } else {
                    log_error("Connection error: " + error.message());
                    handler(false);
                }
            });
    } catch (const std::exception& e) {
        log_error("Resolve error: " + std::string(e.what()));
        handler(false);
    }
}

void TCPClient::disconnect() {
    if (connected_ && socket_->is_open()) {
        boost::system::error_code ec;
        socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_->close(ec);
        connected_ = false;
        log_info("Disconnected from server");
        disconnect_handler_();
    }
}

bool TCPClient::is_connected() const {
    return connected_ && socket_->is_open();
}

void TCPClient::send_data(const uint8_t* data, std::size_t length) {
    if (!is_connected()) {
        log_error("Cannot send: not connected");
        return;
    }
    
    boost::asio::async_write(*socket_, 
        boost::asio::buffer(data, length),
        [this](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
            if (error) {
                log_error("Send error: " + error.message());
                if (error == boost::asio::error::connection_reset ||
                    error == boost::asio::error::broken_pipe) {
                    disconnect();
                }
            }
        });
}

void TCPClient::set_message_handler(MessageHandler handler) {
    message_handler_ = std::move(handler);
}

void TCPClient::set_disconnect_handler(DisconnectHandler handler) {
    disconnect_handler_ = std::move(handler);
}

void TCPClient::start_read() {
    if (!is_connected()) {
        return;
    }
    
    socket_->async_read_some(
        boost::asio::buffer(recv_buffer_),
        [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
            this->handle_read(error, bytes_transferred);
        });
}

void TCPClient::handle_read(const boost::system::error_code& error, 
                          std::size_t bytes_transferred) {
    if (!error) {
        // Call the message handler with binary data for flatbuffer
        message_handler_(recv_buffer_.data(), bytes_transferred);
        
        // Continue reading
        start_read();
    } else if (error == boost::asio::error::eof ||
               error == boost::asio::error::connection_reset) {
        log_info("Server disconnected");
        disconnect();
    } else {
        log_error("Read error: " + error.message());
        disconnect();
    }
}

} // namespace network