#include "net/common.hpp"
#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        running = false;
        network_app::log_info("Shutdown signal received");
    }
}

int main(int argc, char* argv[]) {
}
