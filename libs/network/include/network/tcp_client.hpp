#pragma once

#include <string>
#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include "common.hpp"

namespace network {

class TCPClient {
public:
    using MessageHandler = std::function<void(const uint8_t*, std::size_t)>;
    using ConnectHandler = std::function<void(bool)>;
    using DisconnectHandler = std::function<void()>;

    TCPClient(boost::asio::io_context& io_context);
    ~TCPClient();

    // Connect to server
    void connect(const std::string& host, int port, ConnectHandler handler);

    // Send binary data (for flatbuffers)
    void send_data(const uint8_t* data, std::size_t length);

    // Disconnect from server
    void disconnect();

    // Check if connected
    bool is_connected() const;

    // Set handlers
    void set_message_handler(MessageHandler handler);
    void set_disconnect_handler(DisconnectHandler handler);

private:
    void start_read();
    void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred);

    boost::asio::io_context& io_context_;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
    std::array<uint8_t, MAX_BUFFER_SIZE> recv_buffer_;
    bool connected_;
    MessageHandler message_handler_;
    DisconnectHandler disconnect_handler_;
};

} // namespace network
