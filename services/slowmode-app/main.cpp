#include <iostream>
#include <filesystem>

#include "mav/Network.h"
#include "mav/TCPClient.h"
#include "mav/UDPServer.h"
#include "mav/MessageSet.h"

#include "VelocityLimits.h"
#include "ConnectionHandler.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    std::cout << "Slow mode app" << std::endl;
    std::string path = fs::current_path().string() + "/mavlink/common.xml";
    auto message_set = mav::MessageSet(path);
    float standard_focal_length = 35.0f;

    ConnectionHandler ch(message_set);
    // scale from 0 to 1
    VelocityLimits velocityLimits(message_set, 1.0f, 1.0f, 1.0f);
    // Manual assignments of velocity limits
    if (argc == 4) {
        velocityLimits.setHorizontalSpeed(std::stof(argv[1]));
        velocityLimits.setVerticalSpeed(std::stof(argv[2]));
        velocityLimits.setYawRate(std::stof(argv[3]));
    }

    while(true) {
        // TODO: Update velocity limits based on readings
        // velocityLimits.update(ch);
        float yaw_rate_limit = ch.getFocalLength()/(standard_focal_length * ch.getZoomLevel());
        velocityLimits.setYawRate(yaw_rate_limit);
        auto message = velocityLimits.getMessage();
        ch.connection->send(message);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
