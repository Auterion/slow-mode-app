#ifndef VELOCITYLIMITS_H
#define VELOCITYLIMITS_H

#include <cmath>
#include "mav/MessageSet.h"
#include "ConnectionHandler.h"

class VelocityLimits {
    private:
        float _horizontal_speed;
        float _vertical_speed;
        float _yaw_rate;
        float _max_yaw_rate;
        float _standard_focal_length;
        float _standard_frame_dim;
        float _standard_fov;
        const mav::MessageSet &_message_set;

        float _computeLinearScale(float focal_length, float zoom_level);
        float _computeQuadraticScale(float focal_length, float zoom_level);
        float _computeFOVScale(float focal_length, float zoom_level);

    public:
        VelocityLimits(const mav::MessageSet &message_set, float horizontal_speed, float vertical_speed, float yaw_rate,
                        float standard_focal_length, float standard_frame_dim);
        ~VelocityLimits();

        void computeYawRate(float focal_length, float standard_focal_length, float zoom_level, int mode);
        bool setHorizontalSpeed(float horizontal_speed);
        bool setVerticalSpeed(float vertical_speed);
        bool setYawRate(float yaw_rate);
        bool setYawRateInDegrees(float yaw_rate);

        float getHorizontalSpeed();
        float getVerticalSpeed();
        float getYawRate();
        mav::Message getMessage();
};

#endif // VELOCITYLIMITS_H