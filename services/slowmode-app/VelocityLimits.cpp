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

void VelocityLimits::computeYawRateLimit(float focal_length, float zoom_level, float standard_focal_length, float yaw_rate_limit_scaler) {
    try
    {
        if (focal_length != focal_length || focal_length == 0.f) {
            setYawRate(NAN);
            return;
        } else {
            setYawRate(2 * yaw_rate_limit_scaler * focal_length/(standard_focal_length * zoom_level) - 1);
            return;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        setYawRate(NAN);
    }
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
