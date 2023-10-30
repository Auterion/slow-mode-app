#include "ConnectionHandler.h"

ConnectionHandler::ConnectionHandler(const mav::MessageSet& message_set) : _message_set(message_set) {}

ConnectionHandler::~ConnectionHandler()
{
    _should_exit = true;
    _PM_thread.join();
    _PM_heartbeat_thread.join();
}

void ConnectionHandler::connect()
{
    changeState(App::app_status_code_t::LOADING, "Connecting...", "");
    if (_state_change_callback) {
        SPDLOG_INFO("Connecting...");
        _state_change_callback(App::app_status_code_t::LOADING, "Connecting...", "");
    }
    _runtime = std::make_unique<mav::NetworkRuntime>(_message_set, physical);
    _connection = _runtime->awaitConnection(4000);
    // Handle connection timeout
    SPDLOG_INFO("Connected!");
    changeState(App::app_status_code_t::SUCCESS, "Connected!", "");

    _initPMRequest();
    _PM_thread = std::thread(&ConnectionHandler::_handlePM, this);
    _PM_heartbeat_thread = std::thread(&ConnectionHandler::_monitorPMHeartbeat, this);
}

void ConnectionHandler::_handlePM()
{
    SPDLOG_INFO("Starting Payload Manager handler thread...");

    while (!shouldExit()) {
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
            auto expectation = _connection->expect("CAMERA_INFORMATION");
            _connection->send(*_PM_request);
            try {
                auto res = _connection->receive(expectation, 1000);
                SPDLOG_INFO("Received camera focal length: {}", res["focal_length"].as<float>());
                _focal_legth = res["focal_length"].as<float>();
                _focal_length_set = true;
            } catch (const std::exception& e) {
                SPDLOG_ERROR(e.what());
                continue;
            }
        }

        // Request camera settings first and then monitor changes
        (*_PM_request)["param1"] = _message_set.idForMessage("CAMERA_SETTINGS");
        _connection->send(*_PM_request);
        while (!shouldExit() && pmExists() && _focal_length_set) {
            // Monitor camera settings changes
            auto expectation = _connection->expect("CAMERA_SETTINGS");
            try {
                auto res = _connection->receive(expectation, 1000);
                SPDLOG_INFO("Received camera zoom level: {}", res["zoomLevel"].as<float>());
                _zoom_level = res["zoomLevel"].as<float>();
            } catch (const std::exception& e) {
                if (std::string(e.what()) != "Expected message timed out") {
                    SPDLOG_ERROR(e.what());
                }
            }
        }
    }
}

void ConnectionHandler::_monitorPMHeartbeat()
{
    SPDLOG_INFO("Starting Payload Manager heartbeat monitor...");
    auto last_pm_heartbeat = std::chrono::system_clock::now();

    while (!shouldExit()) {
        auto expectation = _connection->expect("HEARTBEAT");
        try {
            auto res = _connection->receive(expectation, 1000);
            int component_id = static_cast<int>(res.header().componentId());
            if (component_id >= _min_max_target_search.first && component_id <= _min_max_target_search.second) {
                if (!pmExists()) {
                    SPDLOG_INFO("Payload Manager found!");
                    SPDLOG_INFO("Received Payload Manager heartbeat from component id: {}", component_id);
                    _target_component = component_id;
                }
                last_pm_heartbeat = std::chrono::system_clock::now();
            }
        } catch (const std::exception& e) {
            if (std::string(e.what()) != "Expected message timed out") {
                SPDLOG_ERROR(e.what());
            }
        }
        if (pmExists() && std::chrono::system_clock::now() - last_pm_heartbeat > _heartbeat_timeout) {
            SPDLOG_INFO("Payload Manager timeout!");
            _focal_length_set = false;
            _focal_legth = NAN;
            _zoom_level = NAN;
            _target_component = -1;
        }
    }
}

bool ConnectionHandler::_initPMRequest()
{
    _PM_request = std::make_shared<mav::Message>(_message_set.create("COMMAND_LONG"));
    (*_PM_request)["target_system"] = 1;
    (*_PM_request)["target_component"] = 100;
    (*_PM_request)["command"] = _message_set.e("MAV_CMD_REQUEST_MESSAGE");
    (*_PM_request)["param1"] = _message_set.idForMessage("CAMERA_SETTINGS");
    (*_PM_request)["param7"] = 2;
    return true;
}

void ConnectionHandler::sendVelocityLimits(float horizontal_speed, float vertical_speed, float yaw_rate) const
{
    auto message = _message_set.create("VELOCITY_LIMITS");
    message["horizontal_velocity"] = horizontal_speed;
    message["vertical_velocity"] = vertical_speed;
    message["yaw_rate"] = yaw_rate;
    _connection->send(message);
}

void ConnectionHandler::changeState(App::app_status_code_t new_state, std::string_view description, std::string_view error) const
{
    if (_state_change_callback) {
        _state_change_callback(new_state, description, error);
    }
}
