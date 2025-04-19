#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

// Forward declarations for flatbuffer generated types
namespace fgsim {
namespace protocol {
struct Command;
struct Status;
struct HelicopterTelemetry;
struct HelicopterControl;
struct FlightGearConfig;
}
}

namespace protocol {
// Wrapper class for Command messages
class CommandMessage {
public:
    enum class Type {
        Start,
        Stop,
        Pause,
        Resume,
        Reset,
        Configure
    };

    struct Config {
        std::string aircraft;
        std::string airport;
        std::string time_of_day;
        std::string weather;
        std::vector<std::string> additional_args;
    };

    // Create command with type only
    static std::vector<uint8_t> create(Type type);

    // Create command with configuration
    static std::vector<uint8_t> create(Type type, Config const& config);

    // Parse command from binary data
    static bool parse(uint8_t const* data, size_t size, Type& type, Config& config);
};

// Wrapper class for Status messages
class StatusMessage {
public:
    enum class Status {
        Idle,
        Starting,
        Running,
        Paused,
        Stopping,
        Error
    };

    struct StatusInfo {
        Status status;
        uint64_t timestamp;
        std::string message;
        uint64_t uptime;
        float cpu_usage;
        float mem_usage;
    };

    // Create status message
    static std::vector<uint8_t> create(StatusInfo const& info);

    // Parse status from binary data
    static bool parse(uint8_t const* data, size_t size, StatusInfo& info);
};

// Wrapper class for Telemetry messages
class TelemetryMessage {
public:
    struct Telemetry {
        // Position
        double latitude;
        double longitude;
        double altitude;

        // Attitude
        float roll;
        float pitch;
        float heading;

        // Velocities
        float airspeed;
        float vertical_speed;
        float ground_speed;

        // Engine
        float engine_rpm;
        float rotor_rpm;

        // Control inputs
        float collective;
        float cyclic_lat;
        float cyclic_lon;
        float pedals;

        // Environment
        float wind_speed;
        float wind_direction;
        float temperature;

        // System
        uint64_t timestamp;
        float sim_time;
    };

    // Parse telemetry from binary data
    static bool parse(uint8_t const* data, size_t size, Telemetry& telemetry);
};

// Wrapper class for Control messages
class ControlMessage {
public:
    struct Control {
        float collective;
        float cyclic_lat;
        float cyclic_lon;
        float pedals;
        uint64_t timestamp;
    };

    // Create control message
    static std::vector<uint8_t> create(Control const& control);
};
} // namespace protocol
