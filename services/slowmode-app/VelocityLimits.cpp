#include "VelocityLimits.h"

VelocityLimits::VelocityLimits(const mav::MessageSet &message_set, float horizontal_speed, float vertical_speed, float yaw_rate,
                               float standard_focal_length, float standard_frame_dim):
    _message_set(message_set),
    _horizontal_speed(horizontal_speed),
    _vertical_speed(vertical_speed),
    _yaw_rate(yaw_rate),
    _max_yaw_rate(yaw_rate),
    _standard_focal_length(standard_focal_length),
    _standard_frame_dim(standard_frame_dim)
{
    assert(_standard_focal_length > 0);
    assert(_standard_frame_dim > 0);
    _standard_fov = 2 * atan(_standard_frame_dim / (2 * _standard_focal_length));
}

VelocityLimits::~VelocityLimits() {
}

float VelocityLimits::_computeLinearScale(float focal_length, float zoom_level) {
    return _standard_focal_length / (focal_length * zoom_level);
}

float VelocityLimits::_computeQuadraticScale(float focal_length, float zoom_level) {
    return pow(_computeLinearScale(focal_length, zoom_level), 2);
}

float VelocityLimits::_computeFOVScale(float focal_length, float zoom_level) {
    float fov = 2 * atan(_standard_frame_dim / (2 * focal_length * zoom_level));
    // std::cout<<"FOV scale: "<<tan(fov / 2) / tan(_standard_fov / 2)<<std::endl;
    return tan(fov / 2) / tan(_standard_fov / 2);
}

void VelocityLimits::computeYawRate(float focal_length, float zoom_level, float standard_focal_length, int mode) {
    // If Pyaload Manager does not report focal length or zoom level, set yaw rate to default
    if (focal_length != focal_length || focal_length <= 0.f || zoom_level != zoom_level || zoom_level <= 0.f) {
            setYawRate(_max_yaw_rate);
            return;
    }

    float yaw_rate_scale = 1.0f;
    switch (mode) {
        case 0:
            yaw_rate_scale = _computeLinearScale(focal_length, zoom_level);
            break;
        case 1:
            yaw_rate_scale = _computeQuadraticScale(focal_length, zoom_level);
            break;
        case 2:
            yaw_rate_scale = _computeFOVScale(focal_length, zoom_level);
            break;
        default:
            yaw_rate_scale = _computeLinearScale(focal_length, zoom_level);
            break;
    }
    setYawRate(_max_yaw_rate * yaw_rate_scale);
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
    message["horizontal_velocity"] = _horizontal_speed;
    message["vertical_velocity"] = _vertical_speed;
    message["yaw_rate"] = _yaw_rate;
    return message;
}
