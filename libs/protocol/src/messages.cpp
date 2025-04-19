#include "protocol/messages.hpp"
#include "command_generated.h"
#include "status_generated.h"
#include "telemetry_generated.h"
#include <flatbuffers/flatbuffers.h>
#include <chrono>

namespace protocol {

// Helper to get current timestamp in milliseconds
uint64_t get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// CommandMessage implementation
std::vector<uint8_t> CommandMessage::create(Type type) {
    flatbuffers::FlatBufferBuilder builder(1024);

    auto fb_type = static_cast<fgsim::protocol::CommandType>(static_cast<int>(type));
    auto timestamp = get_timestamp();

    auto command = fgsim::protocol::CreateCommand(
        builder,
        fb_type,
        timestamp,
        0  // No config
    );

    builder.Finish(command);

    // Copy to std::vector
    uint8_t* buf = builder.GetBufferPointer();
    size_t size = builder.GetSize();
    return std::vector<uint8_t>(buf, buf + size);
}

std::vector<uint8_t> CommandMessage::create(Type type, const Config& config) {
    flatbuffers::FlatBufferBuilder builder(1024);

    // Create additional args vector
    std::vector<flatbuffers::Offset<flatbuffers::String>> fb_args;
    for (const auto& arg : config.additional_args) {
        fb_args.push_back(builder.CreateString(arg));
    }
    auto args_vec = builder.CreateVector(fb_args);

    // Create config
    auto fb_config = fgsim::protocol::CreateFlightGearConfig(
        builder,
        builder.CreateString(config.aircraft),
        builder.CreateString(config.airport),
        builder.CreateString(config.time_of_day),
        builder.CreateString(config.weather),
        args_vec
    );

    auto fb_type = static_cast<fgsim::protocol::CommandType>(static_cast<int>(type));
    auto timestamp = get_timestamp();

    auto command = fgsim::protocol::CreateCommand(
        builder,
        fb_type,
        timestamp,
        fb_config
    );

    builder.Finish(command);

    // Copy to std::vector
    uint8_t* buf = builder.GetBufferPointer();
    size_t size = builder.GetSize();
    return std::vector<uint8_t>(buf, buf + size);
}

bool CommandMessage::parse(const uint8_t* data, size_t size, Type& type, Config& config) {
    // Verify the buffer
    flatbuffers::Verifier verifier(data, size);
    if (!fgsim::protocol::VerifyCommandBuffer(verifier)) {
        return false;
    }

    auto command = fgsim::protocol::GetCommand(data);

    // Extract command type
    type = static_cast<Type>(static_cast<int>(command->type()));

    // Extract config if available
    if (command->config()) {
        auto fb_config = command->config();

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
            for (const auto& arg : *fb_config->additional_args()) {
                if (arg) {
                    config.additional_args.push_back(arg->str());
                }
            }
        }
    }

    return true;
}

// StatusMessage implementation
std::vector<uint8_t> StatusMessage::create(const StatusInfo& info) {
    flatbuffers::FlatBufferBuilder builder(1024);

    auto fb_status = static_cast<fgsim::protocol::FGStatus>(static_cast<int>(info.status));
    auto message = builder.CreateString(info.message);

    auto status = fgsim::protocol::CreateStatus(
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
    size_t size = builder.GetSize();
    return std::vector<uint8_t>(buf, buf + size);
}

bool StatusMessage::parse(const uint8_t* data, size_t size, StatusInfo& info) {
    // Verify the buffer
    flatbuffers::Verifier verifier(data, size);
    if (!fgsim::protocol::VerifyStatusBuffer(verifier)) {
        return false;
    }

    auto status = fgsim::protocol::GetStatus(data);

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
bool TelemetryMessage::parse(const uint8_t* data, size_t size, Telemetry& telemetry) {
    // Verify the buffer
    flatbuffers::Verifier verifier(data, size);
    if (!fgsim::protocol::VerifyHelicopterTelemetryBuffer(verifier)) {
        return false;
    }

    auto fb_telemetry = fgsim::protocol::GetHelicopterTelemetry(data);

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
std::vector<uint8_t> ControlMessage::create(const Control& control) {
    flatbuffers::FlatBufferBuilder builder(1024);

    auto timestamp = control.timestamp ? control.timestamp : get_timestamp();

    auto fb_control = fgsim::protocol::CreateHelicopterControl(
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
    size_t size = builder.GetSize();
    return std::vector<uint8_t>(buf, buf + size);
}

} // namespace protocol