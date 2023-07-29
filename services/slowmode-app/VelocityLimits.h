#ifndef VELOCITYLIMITS_H
#define VELOCITYLIMITS_H

#include "mav/MessageSet.h"
#include "ConnectionHandler.h"

class VelocityLimits {
    private:
        float _horizontal_speed;
        float _vertical_speed;
        float _yaw_rate;
        const mav::MessageSet &_message_set;

    public:
        VelocityLimits(const mav::MessageSet &message_set, float horizontal_speed, float vertical_speed, float yaw_rate);
        ~VelocityLimits();

        void computeYawRateLimit(float focal_length, float standard_focal_length, float zoom_level, float yaw_rate_limit_scaler);
        bool setHorizontalSpeed(float horizontal_speed);
        bool setVerticalSpeed(float vertical_speed);
        bool setYawRate(float yaw_rate);

        float getHorizontalSpeed();
        float getVerticalSpeed();
        float getYawRate();
        mav::Message getMessage();
};

#endif // VELOCITYLIMITS_H