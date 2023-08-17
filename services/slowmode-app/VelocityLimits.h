#ifndef VELOCITYLIMITS_H
#define VELOCITYLIMITS_H

#include "spdlog/spdlog.h"

#include <cmath>
#include <iostream>

class VelocityLimits {
    private:
        float _horizontal_speed;
        float _vertical_speed;
        float _yaw_rate;
        float _max_yaw_rate;
        float _standard_focal_length;
        float _standard_frame_dim;
        float _standard_fov;
        float _yaw_rate_multiplicator;

        float _computeLinearScale(float focal_length, float zoom_level);
        float _computeQuadraticScale(float focal_length, float zoom_level);

    public:
        VelocityLimits(float horizontal_speed, float vertical_speed, float yaw_rate,
                        float standard_focal_length, float yaw_rate_multiplicator);

        void computeAndUpdateYawRate(float focal_length, float standard_focal_length, float zoom_level, int mode = 0);
        void setHorizontalSpeed(float horizontal_speed);
        void setVerticalSpeed(float vertical_speed);
        void setYawRate(float yaw_rate);
        void setYawRateInDegrees(float yaw_rate);

        float getHorizontalSpeed();
        float getVerticalSpeed();
        float getYawRate();
};

#endif // VELOCITYLIMITS_H