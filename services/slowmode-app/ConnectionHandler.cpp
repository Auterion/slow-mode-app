#include "ConnectionHandler.h"

ConnectionHandler::ConnectionHandler(const mav::MessageSet &message_set) :
    _message_set(message_set)
{
    runtime = std::make_unique<mav::NetworkRuntime>(_message_set, physical);
    connection = runtime->awaitConnection(4000);
    std::cout<<"Connected!" << std::endl;
    initPMRequest();
}

ConnectionHandler::~ConnectionHandler() {
}

bool ConnectionHandler::initPMRequest() {
    // int MAV_CMD_REQUEST_CAMERA_INFORMATION=521;
    // int MAV_CMD_RESET_CAMERA_SETTINGS=529;
    _PM_request = std::make_shared<mav::Message>(_message_set.create("COMMAND_LONG"));
    (*_PM_request)["target_system"] = 1;
    (*_PM_request)["target_component"] = 0;
    (*_PM_request)["command"] = _message_set.e("MAV_CMD_REQUEST_MESSAGE");
    (*_PM_request)["param1"] = _message_set.idForMessage("AUTOPILOT_VERSION");
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

