#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "mav/MessageSet.h"
#include "mav/Message.h"
#include "mav/Network.h"

#include "mav/TCPClient.h"
#include "mav/UDPServer.h"
#include <cmath>

class ConnectionHandler {
    private:
        std::shared_ptr<mav::Message> _PM_request, _PM_request_CAM_SETTINGS;
        #ifdef UDP
            mav::UDPServer physical = mav::UDPServer(14540);
        #else
            mav::TCPClient physical = mav::TCPClient("10.41.1.1", 5790);
        #endif

        std::pair<int, int> _min_max_target_search = {100,106};
        std::thread _PM_thread, _PM_heartbeat_thread;
        const mav::MessageSet &_message_set;
        std::shared_ptr<mav::NetworkRuntime> _runtime;

        int _target_component; // target component of the PM
        // Looks for the target component of the PM 100-106
        int _findTargetComponent();
        void _handlePM();
        void _monitorPMHeartbeat();

        std::atomic<float> _focal_legth = NAN;
        std::atomic<float> _zoom_level = NAN;
        std::atomic<bool> _focal_length_set = false;
        std::atomic<bool> _PM_exists = false;
        float _horizontal_speed = NAN;
        float _vertical_speed = NAN;
        float _yaw_rate = NAN;

        std::chrono::milliseconds _heartbeat_timeout;

    public:
        std::shared_ptr<mav::Connection> connection;
        ConnectionHandler(const mav::MessageSet &message_set);
        ~ConnectionHandler();
        bool initPMRequest();
        std::shared_ptr<mav::Message> getPMRequest();
        float getFocalLength                () const {return _focal_legth;};
        float getZoomLevel                  () const {return _zoom_level;};

        float getHorizontalSpeed            () const {return _horizontal_speed;};
        float getVerticalSpeed              () const {return _vertical_speed;};
        float getYawRate                    () const {return _yaw_rate;};
        void setHorizontalSpeed             (float horizontal_speed) {_horizontal_speed = horizontal_speed;};
        void setVerticalSpeed               (float vertical_speed) {_vertical_speed = vertical_speed;};
        void setYawRate                     (float yaw_rate) {_yaw_rate = yaw_rate;};
};

#endif // CONNECTIONHANDLER_H

