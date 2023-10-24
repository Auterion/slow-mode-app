#include "VelocityLimits.h"

VelocityLimits::VelocityLimits(
    float horizontal_speed, float vertical_speed, float yaw_rate, float standard_focal_length, float yaw_rate_multiplicator) :
    _horizontal_speed(horizontal_speed),
    _vertical_speed(vertical_speed),
    _yaw_rate(yaw_rate),
    _max_yaw_rate(yaw_rate),
    _standard_focal_length(standard_focal_length),
    _yaw_rate_multiplicator(yaw_rate_multiplicator)
{
    if (_standard_focal_length > 0) {
        _standard_focal_length;
    } else {
        _standard_focal_length = NAN;
        SPDLOG_INFO("Invalid standard focal length");
    }
}

float VelocityLimits::_computeLinearScale(float focal_length, float zoom_level)
{
    if (focal_length <= 0.f || zoom_level <= 0.f) {
        SPDLOG_INFO("Invalid focal length or zoom level");
        return 1.0f;
    }
    return _standard_focal_length / (focal_length * zoom_level);
}

float VelocityLimits::_computeQuadraticScale(float focal_length, float zoom_level)
{
    if (focal_length <= 0.f || zoom_level <= 0.f) {
        SPDLOG_INFO("Invalid focal length or zoom level");
        return 1.0f;
    }
    return pow(_computeLinearScale(focal_length, zoom_level), 2);
}

void VelocityLimits::computeAndUpdateYawRate(float focal_length, float zoom_level, float standard_focal_length, int mode)
{
    // If Pyaload Manager does not report focal length or zoom level, set yaw rate
    // to default
    if (focal_length != focal_length || focal_length <= 0.f || zoom_level != zoom_level || zoom_level <= 0.f) {
        setYawRateInDegrees(_max_yaw_rate);
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
        default:
            yaw_rate_scale = _computeLinearScale(focal_length, zoom_level);
            break;
    }
    setYawRateInDegrees(_max_yaw_rate * yaw_rate_scale * _yaw_rate_multiplicator);
}

void VelocityLimits::setHorizontalSpeed(float horizontal_speed)
{
    _horizontal_speed = horizontal_speed;
}

void VelocityLimits::setVerticalSpeed(float vertical_speed)
{
    _vertical_speed = vertical_speed;
}

void VelocityLimits::setYawRate(float yaw_rate)
{
    _yaw_rate = yaw_rate;
}

void VelocityLimits::setYawRateInDegrees(float yaw_rate)
{
    _yaw_rate = yaw_rate * M_PI / 180;
}

float VelocityLimits::getHorizontalSpeed()
{
    return _horizontal_speed;
}

float VelocityLimits::getVerticalSpeed()
{
    return _vertical_speed;
}

float VelocityLimits::getYawRate()
{
    return _yaw_rate;
}
