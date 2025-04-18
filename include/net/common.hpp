#pragma once

#include <string>
#include <iostream>
#include <boost/asio.hpp>

namespace network_app {

// Common constants
constexpr int DEFAULT_TCP_PORT = 8080;
constexpr int DEFAULT_UDP_PORT = 8081;
constexpr int MAX_BUFFER_SIZE = 1024;

// Error handling utility
inline void log_error(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
}

// Success logging utility
inline void log_info(const std::string& message) {
    std::cout << "Info: " << message << std::endl;
}

} // namespace network_app
