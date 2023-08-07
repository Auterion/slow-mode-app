#include "ConnectionHandler.h"

ConnectionHandler::ConnectionHandler(const mav::MessageSet &message_set) :
    _message_set(message_set),
    _heartbeat_timeout(3000)
{
    _runtime = std::make_unique<mav::NetworkRuntime>(_message_set, physical);
    connection = _runtime->awaitConnection(4000);
    std::cout<<"Connected!" << std::endl;

    initPMRequest();
    _PM_thread = std::thread(&ConnectionHandler::_handlePM, this);
    _PM_thread.detach();
    _PM_heartbeat_thread = std::thread(&ConnectionHandler::_monitorPMHeartbeat, this);
    _PM_heartbeat_thread.detach();
}

ConnectionHandler::~ConnectionHandler() {
}

int ConnectionHandler::_findTargetComponent() {
    int curr_target = _min_max_target_search.first;
    auto request = getPMRequest();
    (*_PM_request)["command"] = 512;
    // (*_PM_request)["target_component"] = _min_max_target_search.first;
    std::cout << "Looking for Payload Manager's target component... " << std::endl;

    for (int i = _min_max_target_search.first; i <= _min_max_target_search.second; i++) {
        (*_PM_request)["target_component"] = i;
        auto expectation = connection->expect("COMMAND_ACK");
        connection->send(*request);
        try
        {
            auto res = connection->receive(expectation, 1000);
            std::cout << "Found Payload Manager on: " << i << std::endl;
            return i;
        }
        catch(const std::exception& e)
        {
            // if (std::string(e.what()) != "Expected message timed out")
                std::cerr << e.what() << '\n';
        }
    }
    return -1;
}

void ConnectionHandler::_handlePM() {
    std::cout<<"Starting Payload Manager handler thread..." << std::endl;

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Check if Payload Manager exists
        if (!_PM_exists) {
            continue;
        }

        // Check if focal length is set. If not yet, request it
        if (!_focal_length_set) {
            _target_component = _findTargetComponent();

            if (_target_component == -1) {
                std::cout << "Could not find Payload Manager's target component..." << std::endl;
                continue;
            }

            std::cout<<"test2" << std::endl;
            // Reqeuest camera information to get the focal length
            (*_PM_request)["param1"] = _message_set.idForMessage("CAMERA_INFORMATION");
            auto expectation = connection->expect("CAMERA_INFORMATION");
            connection->send(*_PM_request);
            try
            {
                auto res = connection->receive(expectation, 1000);
                std::cout << "Received camera focal length: " << res["focal_length"].as<float>() << std::endl;
                _focal_legth = res["focal_length"].as<float>();
                _focal_length_set = true;
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                continue;
            }
        }

        // Request camera settings first and then monitor changes
        (*_PM_request)["param1"] = _message_set.idForMessage("CAMERA_SETTINGS");
        connection->send(*_PM_request);
        while (_PM_exists && _focal_length_set) {
            // std::cout<<"test3" << std::endl;
            // Monitor camera settings changes
            auto expectation = connection->expect("CAMERA_SETTINGS");
            try
            {
                auto res = connection->receive(expectation, 1000);
                std::cout<<"Received camera zoom level:" << res["zoomLevel"].as<float>() << std::endl;
                _zoom_level = res["zoomLevel"].as<float>();
            }
            catch(const std::exception& e)
            {
                if (std::string(e.what()) != "Expected message timed out")
                    std::cerr << e.what() << '\n';
            }
        }
    }
}

void ConnectionHandler::_monitorPMHeartbeat() {
    std::cout<<"Starting Payload Manager heartbeat monitor..." << std::endl;
    auto last_pm_heartbeat = std::chrono::system_clock::now();

    while(true) {
        auto expectation = connection->expect("HEARTBEAT");
        try
        {
            auto res = connection->receive(expectation, 1000);
            if (res["type"].as<uint8_t>() == 30) {
                if (!_PM_exists) {
                    _PM_exists = true;
                    std::cout<<"Payload Manager found!" << std::endl;
                }
                // std::cout<<"Received Payload Manager heartbeat" << std::endl;
                last_pm_heartbeat = std::chrono::system_clock::now();
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        if (_PM_exists && std::chrono::system_clock::now() - last_pm_heartbeat > _heartbeat_timeout) {
            std::cout<<"Payload Manager timeout!" << std::endl;
            _PM_exists = false;
            _focal_length_set = false;
            _focal_legth = NAN;
            _zoom_level = NAN;
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
