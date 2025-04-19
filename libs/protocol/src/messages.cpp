#include "protocol/messages.hpp"
#include "command_generated.h"
#include "status_generated.h"
#include "telemetry_generated.h"
#include <flatbuffers/flatbuffers.h>
#include <chrono>

namespace protocol {
// Helper to get current timestamp in milliseconds
uint64_t get_timestamp() {
    auto const now = std::chrono::system_clock::now();
    auto const duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// CommandMessage implementation
std::vector<uint8_t> CommandMessage::create(Type type) {
    flatbuffers::FlatBufferBuilder builder(1024);

    auto const fb_type = static_cast<fgsim::protocol::CommandType>(static_cast<int>(type));
    auto const timestamp = get_timestamp();

    auto const command = fgsim::protocol::CreateCommand(
        builder,
        fb_type,
        timestamp,
        0 // No config
        );

    builder.Finish(command);

    // Copy to std::vector
    uint8_t* buf = builder.GetBufferPointer();
    size_t const size = builder.GetSize();
    return {buf, buf + size};
}

std::vector<uint8_t> CommandMessage::create(Type type, Config const& config) {
    flatbuffers::FlatBufferBuilder builder(1024);

    // Create additional args vector
    std::vector<flatbuffers::Offset<flatbuffers::String>> fb_args;
    for (auto const& arg : config.additional_args) {
        fb_args.push_back(builder.CreateString(arg));
    }
    auto const args_vec = builder.CreateVector(fb_args);

    // Create config
    auto const fb_config = fgsim::protocol::CreateFlightGearConfig(
        builder,
        builder.CreateString(config.aircraft),
        builder.CreateString(config.airport),
        builder.CreateString(config.time_of_day),
        builder.CreateString(config.weather),
        args_vec
        );

    auto const fb_type = static_cast<fgsim::protocol::CommandType>(static_cast<int>(type));
    auto const timestamp = get_timestamp();

    auto const command = fgsim::protocol::CreateCommand(
        builder,
        fb_type,
        timestamp,
        fb_config
        );

    builder.Finish(command);

    // Copy to std::vector
    uint8_t* buf = builder.GetBufferPointer();
    size_t const size = builder.GetSize();
    return {buf, buf + size};
}

bool CommandMessage::parse(uint8_t const* data, size_t size, Type& type, Config& config) {
    // Verify the buffer
    if (flatbuffers::Verifier verifier(data, size); !fgsim::protocol::VerifyCommandBuffer(verifier)) {
        return false;
    }
    auto const command = fgsim::protocol::GetCommand(data);

    // Extract command type
    type = static_cast<Type>(static_cast<int>(command->type()));

    // Extract config if available
    if (command->config()) {
        auto const fb_config = command->config();

        if (fb_config->aircraft()) {
            config.aircraft = fb_config->aircraft()->str();
        }
        if (fb_config->airport()) {
            config.airport = fb_config->airport()->str();
        }
        if (fb_config->time_of_day()) {
            config.time_of_day = fb_config->time_of_day()->str();
        }
        if (fb_config->weather()) {
            config.weather = fb_config->weather()->str();
        }
        // Extract additional args
        config.additional_args.clear();
        if (fb_config->additional_args()) {
            for (auto const& arg : *fb_config->additional_args()) {
                if (arg) {
                    config.additional_args.push_back(arg->str());
                }
            }
        }
    }
    return true;
}

// StatusMessage implementation
std::vector<uint8_t> StatusMessage::create(StatusInfo const& info) {
    flatbuffers::FlatBufferBuilder builder(1024);

    auto const fb_status = static_cast<fgsim::protocol::FGStatus>(static_cast<int>(info.status));
    auto const message = builder.CreateString(info.message);

    auto const status = fgsim::protocol::CreateStatus(
        builder,
        fb_status,
        info.timestamp ? info.timestamp : get_timestamp(),
        message,
        info.uptime,
        info.cpu_usage,
        info.mem_usage
        );

    builder.Finish(status);

    // Copy to std::vector
    uint8_t* buf = builder.GetBufferPointer();
    size_t const size = builder.GetSize();
    return {buf, buf + size};
}

bool StatusMessage::parse(uint8_t const* data, size_t size, StatusInfo& info) {
    // Verify the buffer
    if (flatbuffers::Verifier verifier(data, size); !fgsim::protocol::VerifyStatusBuffer(verifier)) {
        return false;
    }
    auto const status = fgsim::protocol::GetStatus(data);

    // Extract status info
    info.status = static_cast<Status>(static_cast<int>(status->status()));
    info.timestamp = status->timestamp();

    if (status->message()) {
        info.message = status->message()->str();
    } else {
        info.message = "";
    }
    info.uptime = status->uptime();
    info.cpu_usage = status->cpu_usage();
    info.mem_usage = status->mem_usage();

    return true;
}

// TelemetryMessage implementation
bool TelemetryMessage::parse(uint8_t const* data, size_t size, Telemetry& telemetry) {
    // Verify the buffer
    if (flatbuffers::Verifier verifier(data, size); !fgsim::protocol::VerifyHelicopterTelemetryBuffer(verifier)) {
        return false;
    }
    auto const fb_telemetry = fgsim::protocol::GetHelicopterTelemetry(data);

    // Extract all telemetry fields
    telemetry.latitude = fb_telemetry->latitude();
    telemetry.longitude = fb_telemetry->longitude();
    telemetry.altitude = fb_telemetry->altitude();

    telemetry.roll = fb_telemetry->roll();
    telemetry.pitch = fb_telemetry->pitch();
    telemetry.heading = fb_telemetry->heading();

    telemetry.airspeed = fb_telemetry->airspeed();
    telemetry.vertical_speed = fb_telemetry->vertical_speed();
    telemetry.ground_speed = fb_telemetry->ground_speed();

    telemetry.engine_rpm = fb_telemetry->engine_rpm();
    telemetry.rotor_rpm = fb_telemetry->rotor_rpm();

    telemetry.collective = fb_telemetry->collective();
    telemetry.cyclic_lat = fb_telemetry->cyclic_lat();
    telemetry.cyclic_lon = fb_telemetry->cyclic_lon();
    telemetry.pedals = fb_telemetry->pedals();

    telemetry.wind_speed = fb_telemetry->wind_speed();
    telemetry.wind_direction = fb_telemetry->wind_direction();
    telemetry.temperature = fb_telemetry->temperature();

    telemetry.timestamp = fb_telemetry->timestamp();
    telemetry.sim_time = fb_telemetry->sim_time();

    return true;
}

// ControlMessage implementation
std::vector<uint8_t> ControlMessage::create(Control const& control) {
    flatbuffers::FlatBufferBuilder builder(1024);

    auto const timestamp = control.timestamp ? control.timestamp : get_timestamp();

    auto const fb_control = fgsim::protocol::CreateHelicopterControl(
        builder,
        control.collective,
        control.cyclic_lat,
        control.cyclic_lon,
        control.pedals,
        timestamp
        );

    builder.Finish(fb_control);

    // Copy to std::vector
    uint8_t* buf = builder.GetBufferPointer();
    size_t const size = builder.GetSize();
    return {buf, buf + size};
}
} // namespace protocol
