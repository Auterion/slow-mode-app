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

#include "app.h"

namespace fs = std::filesystem;

std::atomic<bool> should_exit(false);

void signal_handler([[maybe_unused]] int signal)
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

void constructStatusDescription(const float horizontalSpeed, const float verticalSpeed, const float yawRate, std::string& description)
{
    if (std::isnan(horizontalSpeed) && std::isnan(verticalSpeed) && std::isnan(yawRate)) {
        description = "No velocity limits set";
    } else {
        description = "Current limits: ";
        if (!std::isnan(horizontalSpeed)) {
            description += fmt::format("Horizontal speed: {:.2f}", horizontalSpeed);
        }
        if (!std::isnan(verticalSpeed)) {
            description += fmt::format(" Vertical speed: {:.2f}", verticalSpeed);
        }
        if (!std::isnan(yawRate)) {
            description += fmt::format(" Yaw rate: {:.2f} deg/s", yawRate / M_PI * 180.0f);
        }
    }
}

int main(int argc, char** argv)
{
    App app;
    app.run();

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
    ch.registerStatusChangeCallback([&app](App::app_status_code_t new_state, std::string_view description, std::string_view error) {
        app.stateCallback(new_state, description, error);
    });
    ch.connect();
    VelocityLimits velocityLimits(NAN, NAN, max_yaw_rate_with_camera, standard_focal_length, yaw_rate_multiplicator);

    std::string description = "";
    std::string error = "";

    constructStatusDescription(NAN, NAN, NAN, description);
    app.stateCallback(App::app_status_code_t::SUCCESS, description, error);

    while (!should_exit) {
        bool updated = false;
        if (ch.pmExists()) {
            updated = velocityLimits.computeAndUpdateYawRate(ch.getFocalLength(), ch.getZoomLevel());
        } else {
            updated = velocityLimits.setYawRateInDegrees(NAN);
        }
        if (updated) {
            constructStatusDescription(
                velocityLimits.getHorizontalSpeed(), velocityLimits.getVerticalSpeed(), velocityLimits.getYawRate(), description);
            app.stateCallback(App::app_status_code_t::SUCCESS, description, error);
        }
        ch.sendVelocityLimits(velocityLimits.getHorizontalSpeed(), velocityLimits.getVerticalSpeed(), velocityLimits.getYawRate());

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
