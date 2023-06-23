#include "VelocityLimits.h"

VelocityLimits::VelocityLimits(const mav::MessageSet &message_set, float horizontal_speed, float vertical_speed, float yaw_rate, float timeout, int type_mask) :
    _message_set(message_set),
    _horizontal_speed(horizontal_speed),
    _vertical_speed(vertical_speed),
    _yaw_rate(yaw_rate),
    _timeout(timeout),
    _type_mask(type_mask)
{
}

VelocityLimits::~VelocityLimits() {
}

bool VelocityLimits::setHorizontalSpeed(float horizontal_speed) {
    _horizontal_speed = horizontal_speed;
    return true;
}

bool VelocityLimits::setVerticalSpeed(float vertical_speed) {
    _vertical_speed = vertical_speed;
    return true;
}

bool VelocityLimits::setYawRate(float yaw_rate) {
    _yaw_rate = yaw_rate;
    return true;
}

bool VelocityLimits::setTimeout(float timeout) {
    _timeout = timeout;
    return true;
}

bool VelocityLimits::setTypeMask(int type_mask) {
    _type_mask = type_mask;
    return true;
}

float VelocityLimits::getHorizontalSpeed() {
    return _horizontal_speed;
}

float VelocityLimits::getVerticalSpeed() {
    return _vertical_speed;
}

float VelocityLimits::getYawRate() {
    return _yaw_rate;
}

float VelocityLimits::getTimeout() {
    return _timeout;
}

int VelocityLimits::getTypeMask() {
    return _type_mask;
}

mav::Message VelocityLimits::getMessage() {
    auto message = _message_set.create("VELOCITY_LIMIT");
    message["horizontal_speed"] = _horizontal_speed;
    message["vertical_speed"] = _vertical_speed;
    message["yaw_rate"] = _yaw_rate;
    message["timeout"] = _timeout;
    message["type_mask"] = _type_mask;
    return message;
}
