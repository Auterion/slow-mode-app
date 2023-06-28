#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "mav/MessageSet.h"
#include "mav/Message.h"
#include "mav/Network.h"

#include "mav/TCPClient.h"
#include "mav/UDPServer.h"

#include <thread>

class ConnectionHandler {
    private:
        std::shared_ptr<mav::Message> _PM_request;
        #ifdef UDP
            mav::UDPServer physical = mav::UDPServer(14540);
        #else
            mav::TCPClient physical = mav::TCPClient("10.41.1.1", 5790);
        #endif
        float _zoom_level; // race cond
        std::thread _PM_thread, _AMC_thread;
        const mav::MessageSet &_message_set;
        std::shared_ptr<mav::NetworkRuntime> runtime;
        
    public:
        std::shared_ptr<mav::Connection> connection;
        ConnectionHandler(const mav::MessageSet &message_set);
        ~ConnectionHandler();
        bool initPMRequest();
        std::shared_ptr<mav::Message> getPMRequest();
        bool startPollingPM();
        bool sendRequest(const mav::Message &request);
        bool getReadings();
};

#endif // CONNECTIONHANDLER_H

