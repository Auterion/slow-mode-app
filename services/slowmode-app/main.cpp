#include <iostream>

#include "mav/Network.h"
#include "mav/TCPClient.h"
#include "mav/UDPServer.h"
#include "mav/MessageSet.h"

#include "VelocityLimits.h"

#include <filesystem>
namespace fs = std::filesystem;

int main(int argc, char** argv) {
    std::cout << "Slow mode app" << std::endl;

    std::string path = fs::current_path().string() + "/mavlink/common.xml";
    auto message_set = mav::MessageSet(path);

    VelocityLimits velocityLimits(message_set, 2.0f, 1.0f, 1.0f, 1.0f, 10);

    // auto physical = mav::TCPClient("10.41.1.1", 5790);
    auto physical = mav::UDPServer(14540);

    mav::NetworkRuntime runtime(message_set, physical);
    auto connection = runtime.awaitConnection(4000);

    while(true) {
        // TODO: Decide which input to base the limits on (zoom, knob, GCS values)
        // velocityLimits.getLimitsFromInput();
        // velocityLimits.setLimits();
        auto message = velocityLimits.getMessage();

        std::cout<<"Sending message..." << std::endl;
        connection->send(message);

        // auto response = connection->receive("VELOCITY_LIMITS", 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
