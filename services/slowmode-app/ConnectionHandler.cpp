#include "ConnectionHandler.h"

ConnectionHandler::ConnectionHandler(const mav::MessageSet &message_set) :
    _message_set(message_set),
    _heartbeat_timeout(3000)
{
    _runtime = std::make_unique<mav::NetworkRuntime>(_message_set, physical);
    connection = _runtime->awaitConnection(4000);
    std::cout<<"Connected!" << std::endl;

    _initPMRequest();
    _PM_thread = std::thread(&ConnectionHandler::_handlePM, this);
    _PM_thread.detach();
    _PM_heartbeat_thread = std::thread(&ConnectionHandler::_monitorPMHeartbeat, this);
    _PM_heartbeat_thread.detach();
}

ConnectionHandler::~ConnectionHandler() {
}

void ConnectionHandler::_handlePM() {
    std::cout<<"Starting Payload Manager handler thread..." << std::endl;

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Check if Payload Manager exists
        if (!pmExists()) {
            continue;
        }

        // Check if focal length is set. If not yet, request it
        if (!_focal_length_set) {
            // Reqeuest camera information to get the focal length
            (*_PM_request)["param1"] = _message_set.idForMessage("CAMERA_INFORMATION");
            (*_PM_request)["target_component"] = static_cast<int>(_target_component);
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
        while (pmExists() && _focal_length_set) {
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
            int component_id = static_cast<int>(res.header().componentId());
            if (component_id >= _min_max_target_search.first && component_id <= _min_max_target_search.second) {
                if (!pmExists()) {
                    std::cout<<"Payload Manager found!" << std::endl;
                    std::cout<<"Received Payload Manager heartbeat from component id:" << component_id << std::endl;
                    _target_component = component_id;
                }
                last_pm_heartbeat = std::chrono::system_clock::now();
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        if (pmExists() && std::chrono::system_clock::now() - last_pm_heartbeat > _heartbeat_timeout) {
            std::cout<<"Payload Manager timeout!" << std::endl;
            _focal_length_set = false;
            _focal_legth = NAN;
            _zoom_level = NAN;
            _target_component = -1;
        }
    }
}

bool ConnectionHandler::_initPMRequest() {
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
