#pragma once

#include <memory>
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include "network/common.hpp"

namespace network {
class UDPClient {
public:
    using MessageHandler = std::function<void(uint8_t const*, std::size_t,
                                              boost::asio::ip::udp::endpoint const&)>;

    explicit UDPClient(boost::asio::io_context& io_context, int local_port = 0);
    ~UDPClient();

    // Start receiving data
    void start();

    // Stop receiving data
    void stop();

    // Send data to a specific endpoint
    void send_data(uint8_t const* data, std::size_t length,
                   boost::asio::ip::udp::endpoint const& endpoint);

    // Send data to a specific host and port
    void send_data(uint8_t const* data, std::size_t length,
                   std::string const& host, int port);

    // Set handler for received messages
    void set_message_handler(MessageHandler handler);

    // Get local port
    [[nodiscard]] int get_local_port() const;

private:
    void start_receive();
    void handle_receive(boost::system::error_code const& error, std::size_t bytes_transferred);

    boost::asio::io_context& io_context_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    std::array<uint8_t, MAX_BUFFER_SIZE> recv_buffer_;
    bool running_;
    MessageHandler message_handler_;
};
} // namespace network
