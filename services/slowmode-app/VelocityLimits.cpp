#include "VelocityLimits.h"

VelocityLimits::VelocityLimits(
    float horizontal_speed, float vertical_speed, float yaw_rate, float standard_focal_length, float yaw_rate_multiplicator) :
    _horizontal_speed(horizontal_speed),
    _vertical_speed(vertical_speed),
    _max_yaw_rate(yaw_rate),
    _standard_focal_length(standard_focal_length),
    _yaw_rate_multiplicator(yaw_rate_multiplicator)
{
    if (_standard_focal_length <= 0) {
        _standard_focal_length = NAN;
        SPDLOG_INFO("Invalid standard focal length");
    }
    _yaw_rate = NAN;
}

float VelocityLimits::_computeLinearScale(float focal_length, float zoom_level) const
{
    if (focal_length <= 0.f || zoom_level <= 0.f) {
        SPDLOG_INFO("Invalid focal length or zoom level");
        return 1.0f;
    }
    return _standard_focal_length / (focal_length * zoom_level);
}

float VelocityLimits::_computeQuadraticScale(float focal_length, float zoom_level) const
{
    if (focal_length <= 0.f || zoom_level <= 0.f) {
        SPDLOG_INFO("Invalid focal length or zoom level");
        return 1.0f;
    }
    return pow(_computeLinearScale(focal_length, zoom_level), 2);
}

bool VelocityLimits::computeAndUpdateYawRate(float focal_length, float zoom_level, int mode)
{
    // If Pyaload Manager does not report focal length or zoom level, set yaw rate
    // to default
    if (std::isnan(focal_length) || focal_length <= 0.f || std::isnan(zoom_level) || zoom_level <= 0.f) {
        return setYawRateInDegrees(_max_yaw_rate);
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
    return setYawRateInDegrees(_max_yaw_rate * yaw_rate_scale * _yaw_rate_multiplicator);
}

bool VelocityLimits::setHorizontalSpeed(float horizontal_speed)
{
    if (abs(_horizontal_speed - horizontal_speed) > 1e-5) {
        _horizontal_speed = horizontal_speed;
        return true;
    }
    return false;
}

bool VelocityLimits::setVerticalSpeed(float vertical_speed)
{
    if (abs(_vertical_speed - vertical_speed) > 1e-5) {
        _vertical_speed = vertical_speed;
        return true;
    }
    return false;
}

bool VelocityLimits::setYawRate(float yaw_rate)
{
    if (std::isnan(yaw_rate) && std::isnan(_yaw_rate)) {
        return false;
    } else if (std::isnan(yaw_rate) != std::isnan(_yaw_rate) || fabs(_yaw_rate - yaw_rate) > 1e-5) {
        _yaw_rate = yaw_rate;
        return true;
    }
    return false;
}

bool VelocityLimits::setYawRateInDegrees(float yaw_rate)
{
    return setYawRate(yaw_rate * float(M_PI) / 180);
}

float VelocityLimits::getHorizontalSpeed() const
{
    return _horizontal_speed;
}

float VelocityLimits::getVerticalSpeed() const
{
    return _vertical_speed;
}

float VelocityLimits::getYawRate() const
{
    return _yaw_rate;
}
