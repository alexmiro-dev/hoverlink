#pragma once

#include <string>
#include <iostream>
#include <functional>
#include <boost/asio.hpp>

namespace network {
// Common constants
constexpr int DEFAULT_TCP_PORT = 5502;
constexpr int DEFAULT_UDP_PORT = 5501;
constexpr int MAX_BUFFER_SIZE = 65536; // Increased buffer size for flatbuffer messages

// Error handling utility
inline void log_error(std::string const& message) {
    std::cerr << "Error: " << message << std::endl;
}

// Success logging utility
inline void log_info(std::string const& message) {
    std::cout << "Info: " << message << std::endl;
}

// Debug logging utility
inline void log_debug(std::string const& message) {
    std::cout << "Debug: " << message << std::endl;
}
} // namespace network
