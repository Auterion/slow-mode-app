#include "ConnectionHandler.h"

ConnectionHandler::ConnectionHandler(const mav::MessageSet &message_set) :
    _message_set(message_set)
{
    runtime = std::make_unique<mav::NetworkRuntime>(_message_set, physical);
    connection = runtime->awaitConnection(4000);

    std::cout<<"Connected!" << std::endl;
    initPMRequest();
    _PM_thread = std::thread(&ConnectionHandler::_handlePM, this);
    _PM_thread.detach();
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
    (*_PM_request)["param1"] = _message_set.idForMessage("CAMERA_INFORMATION");
    auto expectation = connection->expect("CAMERA_INFORMATION");
    connection->send(*_PM_request);
    auto res = connection->receive(expectation, 1000);
    std::cout << "Received camera focal length: " << res["focal_length"].as<float>() << std::endl;

    auto request = getPMRequest();
    connection->send(*request);

    while(true) {
        auto expectation = connection->expect("CAMERA_SETTINGS");
        try
        {
            auto res = connection->receive(expectation, 1000);
            std::cout<<"Received response:" << res["zoomLevel"].as<float>() << std::endl;
        }
        catch(const std::exception& e)
        {
            if (std::string(e.what()) != "Expected message timed out")
                std::cerr << e.what() << '\n';
        }
    }
}

bool ConnectionHandler::initPMRequest() {
    // int MAV_CMD_REQUEST_CAMERA_INFORMATION=521;
    // int MAV_CMD_RESET_CAMERA_SETTINGS=529;
    _PM_request = std::make_shared<mav::Message>(_message_set.create("COMMAND_LONG"));
    (*_PM_request)["target_system"] = 1;
    (*_PM_request)["target_component"] = 100;
    (*_PM_request)["command"] = _message_set.e("MAV_CMD_REQUEST_MESSAGE");
    // (*_PM_request)["param1"] = _message_set.idForMessage("AUTOPILOT_VERSION");
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

