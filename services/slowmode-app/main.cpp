#include <filesystem>
#include <iostream>

#include "spdlog/cfg/env.h"
#include "spdlog/spdlog.h"

#include "mav/MessageSet.h"
#include "mav/Network.h"
#include "mav/TCPClient.h"
#include "mav/UDPServer.h"

#include "ConnectionHandler.h"
#include "VelocityLimits.h"

namespace fs = std::filesystem;

std::atomic<bool> should_exit(false);

void signal_handler(int signal)
{
    should_exit = true;
}

std::string getEnvVar(std::string const& key)
{
    const char* val = getenv(key.c_str());
    if (val == NULL) {
        SPDLOG_ERROR("Environment variable {} not found", key);
        throw std::invalid_argument("Environment variable " + key + " not found \n");
    }
    SPDLOG_INFO("Environment variable {} found with value: {}", key, val);
    return val;
}

void manualBroadcast(VelocityLimits& velocityLimits, ConnectionHandler& ch, char** argv)
{
    SPDLOG_INFO("Manual velocity limits");
    velocityLimits.setHorizontalSpeed(std::stof(argv[1]));
    velocityLimits.setVerticalSpeed(std::stof(argv[2]));
    velocityLimits.setYawRateInDegrees(std::stof(argv[3]));

    while (!should_exit) {
        ch.sendVelocityLimits(velocityLimits.getHorizontalSpeed(), velocityLimits.getVerticalSpeed(), velocityLimits.getYawRate());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return;
}

int main(int argc, char** argv)
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    spdlog::cfg::load_env_levels();
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%7l%$] [%20!s:%-3#] %v");
    SPDLOG_INFO("Slow mode app");

    std::string path = fs::current_path().string() + "/mavlink/auterion.xml";
    auto message_set = mav::MessageSet(path);
    float standard_focal_length = 24.0f; // A7R
    float max_yaw_rate_with_camera = 45.0f; // deg/s
    float yaw_rate_multiplicator = 1.0f;

    try {
        yaw_rate_multiplicator = std::stof(getEnvVar("YAW_RATE_MULTIPLICATOR"));
    } catch (const std::invalid_argument& e) {
        SPDLOG_ERROR(e.what());
        yaw_rate_multiplicator = 1.0f;
    }

    try {
        max_yaw_rate_with_camera = std::stof(getEnvVar("MAX_YAW_RATE"));
    } catch (const std::invalid_argument& e) {
        SPDLOG_ERROR(e.what());
        max_yaw_rate_with_camera = 45.0f;
    }

    SPDLOG_INFO("Max yaw rate: {}", max_yaw_rate_with_camera);
    SPDLOG_INFO("Yaw rate multiplicator: {}", yaw_rate_multiplicator);

    SPDLOG_INFO("Starting connection handler...");

    ConnectionHandler ch(message_set);
    // ch = std::make_unique<ConnectionHandler>(message_set);
    VelocityLimits velocityLimits(NAN, NAN, max_yaw_rate_with_camera, standard_focal_length, yaw_rate_multiplicator);

    // Manual assignments of velocity limits if 3 arguments are passed
    if (argc == 4) {
        manualBroadcast(velocityLimits, ch, argv);
        return 0;
    }

    while (!should_exit) {
        if (ch.pmExists()) {
            velocityLimits.computeAndUpdateYawRate(ch.getFocalLength(), ch.getZoomLevel(), standard_focal_length);
        } else {
            velocityLimits.setYawRateInDegrees(NAN);
        }
        ch.sendVelocityLimits(velocityLimits.getHorizontalSpeed(), velocityLimits.getVerticalSpeed(), velocityLimits.getYawRate());

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
