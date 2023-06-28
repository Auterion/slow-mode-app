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

    ConnectionHandler ch(message_set);
    VelocityLimits velocityLimits(message_set, 1.0f, 1.0f, 1.0f);
    if (argc == 4) {
        velocityLimits.setHorizontalSpeed(std::stof(argv[1]));
        velocityLimits.setVerticalSpeed(std::stof(argv[2]));
        velocityLimits.setYawRate(std::stof(argv[3]));
    }

    // auto expectation = ch.connection->expect("COMMAND_ACK");
    // auto request = ch.getPMRequest();

    // std::cout<<"Sending request:" << request->toString() << std::endl;
    // ch.connection->send(*request);
    // auto res = ch.connection->receive(expectation, 3000);

    std::cout<<"Sending velocity limits:" << velocityLimits.getMessage().toString() << std::endl;

    while(true) {
        // TODO: Update velocity limits based on readings
        // velocityLimits.update(ch.getReadings());
        auto message = velocityLimits.getMessage();

        std::cout<<"Sending message..." << std::endl;
        ch.connection->send(message);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}
