#include <iostream>
#include <filesystem>

#include "mav/Network.h"
#include "mav/TCPClient.h"
#include "mav/UDPServer.h"
#include "mav/MessageSet.h"

#include "VelocityLimits.h"
#include "ConnectionHandler.h"

namespace fs = std::filesystem;

std::string getEnvVar(std::string const & key )
{
    const char * val = getenv(key.c_str() );
    if (val == NULL) {
        throw std::invalid_argument("Environment variable " + key + " not found");
    }
    std::cout<<"Environment variable "<<key<<" found with value "<<val<<std::endl;
    return val;
}

int main(int argc, char** argv) {
    std::cout << "Slow mode app" << std::endl;
    std::string path = fs::current_path().string() + "/mavlink/common.xml";
    auto message_set = mav::MessageSet(path);
    float standard_focal_length = 35.0f;

    ConnectionHandler ch(message_set);
    // scale from -1 to 1
    VelocityLimits velocityLimits(message_set, 1.0f, 1.0f, 1.0f);

    float yaw_rate_limit_scaler = 1.0f;
    try {
        yaw_rate_limit_scaler = std::stof(getEnvVar("YAW_RATE_MULTIPLICATOR"));
    }
    catch (const std::invalid_argument& e) {
        std::cerr << e.what();
        yaw_rate_limit_scaler = 1.0f;
    }

    // Manual assignments of velocity limits
    if (argc == 4) {
        velocityLimits.setHorizontalSpeed(std::stof(argv[1]));
        velocityLimits.setVerticalSpeed(std::stof(argv[2]));
        velocityLimits.setYawRate(std::stof(argv[3]));

        while(true) {
            auto message = velocityLimits.getMessage();
            ch.connection->send(message);

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    while(true) {
        velocityLimits.computeYawRateLimit(ch.getFocalLength(), ch.getZoomLevel(), standard_focal_length, yaw_rate_limit_scaler);
        auto message = velocityLimits.getMessage();
        ch.connection->send(message);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
