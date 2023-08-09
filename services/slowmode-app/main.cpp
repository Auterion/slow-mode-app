#include <iostream>
#include <filesystem>

#include "mav/Network.h"
#include "mav/TCPClient.h"
#include "mav/UDPServer.h"
#include "mav/MessageSet.h"

#include "VelocityLimits.h"
#include "ConnectionHandler.h"

namespace fs = std::filesystem;

enum Modes {
    linear = 0,
    quadratic,
    FOV
};

std::string getEnvVar(std::string const & key) {
    const char * val = getenv(key.c_str());
    if (val == NULL) {
        throw std::invalid_argument("Environment variable " + key + " not found \n");
    }
    std::cout<<"Environment variable "<<key<<" found with value "<<val<<std::endl;
    return val;
}

int getScalingMode(std::string const & key) {
    const char * val = getenv(key.c_str());
    int mode;
    try
    {
        if (val == NULL) {
            throw std::invalid_argument("Environment variable " + key + " not found \n");
        }
        std::cout<<"Environment variable "<<key<<" found with value "<<val<<std::endl;
        mode = std::stoi(getEnvVar("SCALING_MODE"));
        if (mode != Modes::linear && mode != Modes::quadratic && mode != Modes::FOV) {
            std::cout<<"Incorrect scaling mode! Set to linear"<<std::endl;
            return(Modes::linear);
        } else {
            std::cout<<"Scaling mode: "<<mode<<std::endl;
            return(mode);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return(Modes::FOV);
}

void manualBroadcast(VelocityLimits& velocityLimits, ConnectionHandler& ch, char** argv) {
    std::cout<<"Manual velocity limits"<<std::endl;
    velocityLimits.setHorizontalSpeed(std::stof(argv[1]));
    velocityLimits.setVerticalSpeed(std::stof(argv[2]));
    velocityLimits.setYawRateInDegrees(std::stof(argv[3]));

    while(true) {
        auto message = velocityLimits.getMessage();
        ch.connection->send(message);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main(int argc, char** argv) {
    std::cout << "Slow mode app" << std::endl;
    std::string path = fs::current_path().string() + "/mavlink/auterion.xml";
    auto message_set = mav::MessageSet(path);
    float standard_focal_length = 24.0f; //A7R
    float standard_frame_dim = 43.9f; //A7R
    float max_yaw_rate_with_camera = 45.0f; //deg/s
    float yaw_rate_multiplicator = 1.0f;

    try {
        yaw_rate_multiplicator = std::stof(getEnvVar("YAW_RATE_MULTIPLICATOR"));
    }
    catch (const std::invalid_argument& e) {
        std::cerr << e.what();
        yaw_rate_multiplicator = 1.0f;
    }

    try {
        max_yaw_rate_with_camera = std::stof(getEnvVar("MAX_YAW_RATE"));
    }
    catch (const std::invalid_argument& e) {
        std::cerr << e.what();
        max_yaw_rate_with_camera = 45.0f;
    }

    int mode = getScalingMode("SCALING_MODE");
    std::cout<<"Scaling mode: "<<mode<<std::endl;
    std::cout<<"Max yaw rate: "<<max_yaw_rate_with_camera<<std::endl;
    std::cout<<"Yaw rate multiplicator: "<<yaw_rate_multiplicator<<std::endl;
    
    ConnectionHandler ch(message_set);
    VelocityLimits velocityLimits(message_set, NAN, NAN, max_yaw_rate_with_camera, standard_focal_length, standard_frame_dim);

    // Manual assignments of velocity limits if 3 arguments are passed
    if (argc == 4) {
        manualBroadcast(velocityLimits, ch, argv);
    }


    while(true) {
        if (ch.getPMExists()) {
            velocityLimits.computeYawRate(ch.getFocalLength(), ch.getZoomLevel(), standard_focal_length, mode);
        } else {
            velocityLimits.setYawRateInDegrees(NAN);
        }
        velocityLimits.setYawRate(velocityLimits.getYawRate() * yaw_rate_multiplicator);
        auto message = velocityLimits.getMessage();
        ch.connection->send(message);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
