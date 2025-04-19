#include "network/udp_client.hpp"
#include <iostream>

namespace network {
UDPClient::UDPClient(boost::asio::io_context& io_context, int local_port)
    : io_context_(io_context),
      socket_(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), local_port)),
      running_(false),
      message_handler_([](uint8_t const*, std::size_t, boost::asio::ip::udp::endpoint const&) {
      }) {
    log_info("UDP client initialized on local port " + std::to_string(get_local_port()));
}

UDPClient::~UDPClient() {
    stop();
}

void UDPClient::start() {
    if (!running_) {
        running_ = true;
        start_receive();
        log_info("UDP client started");
    }
}

void UDPClient::stop() {
    if (running_) {
        running_ = false;
        boost::system::error_code ec;
        std::ignore = socket_.close(ec);
        log_info("UDP client stopped");
    }
}

void UDPClient::send_data(uint8_t const* data, std::size_t length,
    boost::asio::ip::udp::endpoint const& endpoint) {
    socket_.async_send_to(
        boost::asio::buffer(data, length),
        endpoint,
        [](boost::system::error_code const& error, std::size_t /*bytes_sent*/) {
            if (error) {
                log_error("Failed to send UDP data: " + error.message());
            }
        });
}

void UDPClient::send_data(uint8_t const* data, std::size_t length,
    std::string const& host, int port) {
    try {
        boost::asio::ip::udp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host, std::to_string(port));

        if (endpoints.begin() != endpoints.end()) {
            auto endpoint = *endpoints.begin();
            send_data(data, length, endpoint.endpoint());
        } else {
            log_error("Could not resolve host: " + host);
        }
    } catch (std::exception const& e) {
        log_error("Error resolving host: " + std::string(e.what()));
    }
}

void UDPClient::set_message_handler(MessageHandler handler) {
    message_handler_ = std::move(handler);
}

int UDPClient::get_local_port() const {
    return socket_.local_endpoint().port();
}

void UDPClient::start_receive() {
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_),
        remote_endpoint_,
        [this](boost::system::error_code const& error, std::size_t bytes_transferred) {
            this->handle_receive(error, bytes_transferred);
        });
}

void UDPClient::handle_receive(boost::system::error_code const& error,
    std::size_t bytes_transferred) {
    if (!error) {
        // Call the message handler with binary data
        message_handler_(recv_buffer_.data(), bytes_transferred, remote_endpoint_);
    } else if (error != boost::asio::error::operation_aborted) {
        log_error("UDP receive error: " + error.message());
    }

    // Continue receiving if still running
    if (running_) {
        start_receive();
    }
}
} // namespace network
