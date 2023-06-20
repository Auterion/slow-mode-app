#ifndef VELOCITYLIMITS_H
#define VELOCITYLIMITS_H

#include "mav/MessageSet.h"

class VelocityLimits {
    private:
        float _horizontal_speed;
        float _vertical_speed;
        float _yaw_rate;
        float _timeout;
        int _type_mask;
        const mav::MessageSet &_message_set;

    public:
        VelocityLimits(const mav::MessageSet &message_set, float horizontal_speed, float vertical_speed, float yaw_rate, float timeout, int type_mask);
        ~VelocityLimits();
        float getHorizontalSpeed();
        float getVerticalSpeed();
        float getYawRate();
        float getTimeout();
        int getTypeMask();
        mav::Message getMessage();
};

#endif // VELOCITYLIMITS_H