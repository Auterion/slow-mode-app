#include "ConnectionHandler.h"

ConnectionHandler::ConnectionHandler(const mav::MessageSet &message_set) :
    _message_set(message_set)
{
    runtime = std::make_unique<mav::NetworkRuntime>(_message_set, physical);
    connection = runtime->awaitConnection(4000);
    std::cout<<"Connected!" << std::endl;

    initPMRequest();
    _PM_thread = std::thread(&ConnectionHandler::_handlePM, this);
    // _PX4_thread = std::thread(&ConnectionHandler::_handlePX4, this);
    // _GCS_thread = std::thread(&ConnectionHandler::_handleGCS, this);
    _PM_thread.detach();
    // _PX4_thread.detach();
    // _GCS_thread.detach();
}

ConnectionHandler::~ConnectionHandler() {
}

int ConnectionHandler::_findTargetComponent() {
    int curr_target = _min_max_target_search.first;
    auto request = getPMRequest();
    (*_PM_request)["command"] = 512;
    (*_PM_request)["target_component"] = _min_max_target_search.first;
    auto expectation = connection->expect("COMMAND_ACK");
    std::cout << "Looking for Payload Manager's target component... " << std::endl;

    while (true) {
        (*_PM_request)["target_component"] = curr_target;
        connection->send(*request);
        try
        {
            auto res = connection->receive(expectation, 500);
            std::cout << "Found Payload Manager on: " << curr_target << std::endl;
            return curr_target;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        curr_target >= _min_max_target_search.second ? curr_target = _min_max_target_search.first : curr_target++;
    }
}

void ConnectionHandler::_handlePM() {
    std::cout<<"Starting PM thread..." << std::endl;
    _target_component = _findTargetComponent();
    (*_PM_request)["target_component"] = _target_component;

    // Reqeuest camera information first to get the focal length
    (*_PM_request)["param1"] = _message_set.idForMessage("CAMERA_INFORMATION");
    auto expectation = connection->expect("CAMERA_INFORMATION");
    connection->send(*_PM_request);
    try
    {
        auto res = connection->receive(expectation, 1000);
        std::cout << "Received camera focal length: " << res["focal_length"].as<float>() << std::endl;
        // TODO: Thread safety
        _focal_legth = res["focal_length"].as<float>();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    // Request camera settings
    (*_PM_request)["param1"] = _message_set.idForMessage("CAMERA_SETTINGS");
    connection->send(*_PM_request);

    while(true) {
        // Monitor camera settings changes
        auto expectation = connection->expect("CAMERA_SETTINGS");
        try
        {
            auto res = connection->receive(expectation, 1000);
            std::cout<<"Received camera zoom level:" << res["zoomLevel"].as<float>() << std::endl;
            // TODO: Thread safety
            _zoom_level = res["zoomLevel"].as<float>();

        }
        catch(const std::exception& e)
        {
            if (std::string(e.what()) != "Expected message timed out")
                std::cerr << e.what() << '\n';
        }
    }
}

void ConnectionHandler::_handlePX4() {
    std::cout<<"Starting PX4 thread..." << std::endl;
    while(true) {
        try
        {
            auto expectation = connection->expect("HEARTBEAT", 1, 1);
            auto res = connection->receive(expectation, 1000);
            // std::cout<<"Received PX4 heartbeat:" << std::endl<< res.toString() << std::endl;

            // TODO: Make it thread safe
            if (res["custom_mode"].as<int>() == 196608) {
                std::cout<<"PX4 is in position mode" << std::endl;
                _position_mode = true;
            }
            else
                _position_mode = false;
        }
        catch(const std::exception& e)
        {
            if (std::string(e.what()) != "Expected message timed out")
                std::cerr << e.what() << '\n';
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void ConnectionHandler::_handleGCS() {
    std::cout<<"Starting GCS thread..." << std::endl;
    while(true) {

        auto expectation_sticks_responsiveness = connection->expect("STICKS_RESPONSIVENESS");
        try
        {
            auto res = connection->receive(expectation_sticks_responsiveness, 1000);
            _responsiveness_level = res["responsiveness"].as<float>();
            // std::cout<<"Received GCS sticks responsiveness:" << res.toString() << std::endl;
        }
        catch(const std::exception& e)
        {
            std::cerr << "Responsiveness:" << e.what() << '\n';
        }
        
        auto expectation_vel_limits = connection->expect("VELOCITY_LIMITS");
        try
        {
            auto res = connection->receive(expectation_vel_limits, 1000);
            // std::cout<<"Received GCS velocity limits:" << res.toString() << std::endl;
            setHorizontalSpeed(res["horizontal_velocity_limit"].as<float>());
            setVerticalSpeed(res["vertical_velocity_limit"].as<float>());
            setYawRate(res["yaw_rate_limit"].as<float>());
        }
        catch(const std::exception& e)
        {
            std::cerr << "Vel_limits:" << e.what() << '\n';
        }
    }

}

bool ConnectionHandler::initPMRequest() {
    _PM_request = std::make_shared<mav::Message>(_message_set.create("COMMAND_LONG"));
    (*_PM_request)["target_system"] = 1;
    (*_PM_request)["target_component"] = 100;
    (*_PM_request)["command"] = _message_set.e("MAV_CMD_REQUEST_MESSAGE");
    (*_PM_request)["param1"] = _message_set.idForMessage("CAMERA_SETTINGS");
    (*_PM_request)["param7"] = 2;
    return true;
}

std::shared_ptr<mav::Message> ConnectionHandler::getPMRequest() {
    return _PM_request;
}

bool ConnectionHandler::startPollingPM() {
    return true;
}

bool ConnectionHandler::sendRequest(const mav::Message &request) {
    return true;
}

bool ConnectionHandler::getReadings() {

    return true;
}

