#pragma once

#include <memory>
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include "network/common.hpp"

namespace network {

    class UDPClient {
    public:
        using MessageHandler = std::function<void(const uint8_t*, std::size_t,
                                               const boost::asio::ip::udp::endpoint&)>;

        UDPClient(boost::asio::io_context& io_context, int local_port = 0);
        ~UDPClient();

        // Start receiving data
        void start();

        // Stop receiving data
        void stop();

        // Send data to a specific endpoint
        void send_data(const uint8_t* data, std::size_t length,
                      const boost::asio::ip::udp::endpoint& endpoint);

        // Send data to a specific host and port
        void send_data(const uint8_t* data, std::size_t length,
                      const std::string& host, int port);

        // Set handler for received messages
        void set_message_handler(MessageHandler handler);

        // Get local port
        int get_local_port() const;

    private:
        void start_receive();
        void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);

        boost::asio::io_context& io_context_;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint remote_endpoint_;
        std::array<uint8_t, MAX_BUFFER_SIZE> recv_buffer_;
        bool running_;
        MessageHandler message_handler_;
    };

} // namespace network