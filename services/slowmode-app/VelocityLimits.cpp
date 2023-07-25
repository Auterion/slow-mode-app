#include "VelocityLimits.h"

VelocityLimits::VelocityLimits(const mav::MessageSet &message_set, float horizontal_speed, float vertical_speed, float yaw_rate) :
    _message_set(message_set),
    _horizontal_speed(horizontal_speed),
    _vertical_speed(vertical_speed),
    _yaw_rate(yaw_rate)
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

float VelocityLimits::getHorizontalSpeed() {
    return _horizontal_speed;
}

float VelocityLimits::getVerticalSpeed() {
    return _vertical_speed;
}

float VelocityLimits::getYawRate() {
    return _yaw_rate;
}

mav::Message VelocityLimits::getMessage() {
    auto message = _message_set.create("VELOCITY_LIMITS");
    message["horizontal_velocity_limit"] = _horizontal_speed;
    message["vertical_velocity_limit"] = _vertical_speed;
    message["yaw_rate_limit"] = _yaw_rate;
    return message;
}

bool VelocityLimits::update(const ConnectionHandler &ch) {
    // auto tmp = ch.getHorizontalSpeed();
    setHorizontalSpeed(ch.getHorizontalSpeed());
    setVerticalSpeed(ch.getVerticalSpeed());
    setYawRate(ch.getYawRate());
    return true;
}
