#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "mav/MessageSet.h"
#include "mav/Message.h"
#include "mav/Network.h"

#include "mav/TCPClient.h"
#include "mav/UDPServer.h"

#include "spdlog/spdlog.h"

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

        std::atomic<int> _target_component = -1; // target component of the PM
        void _handlePM();
        void _monitorPMHeartbeat();
        bool _initPMRequest();

        std::atomic<float> _focal_legth = NAN;
        std::atomic<float> _zoom_level = NAN;
        std::atomic<bool> _focal_length_set = false;
        std::atomic<bool> _should_exit = false;

        std::chrono::milliseconds _heartbeat_timeout{3000};
        std::shared_ptr<mav::Connection> _connection;
    public:
        ConnectionHandler(const mav::MessageSet &message_set);
        ~ConnectionHandler();
        void sendVelocityLimits(float horizontal_speed, float vertical_speed, float yaw_rate);
        float getFocalLength                () const {return _focal_legth;};
        float getZoomLevel                  () const {return _zoom_level;};
        bool pmExists                       () const {return _target_component == -1;};
        bool shouldExit                     () const {return _should_exit;};
        void close                          ()       {_should_exit = true;};
};

#endif // CONNECTIONHANDLER_H

