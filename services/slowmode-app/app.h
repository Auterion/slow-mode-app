#pragma once

// #include "ntrip_client.h"

#include <string>
#include <thread>
#include <restinio/all.hpp>
#include <csignal>
#include <iostream>
#include <magic_enum.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/bin_to_hex.h>

class App {
private:
    std::mutex _state_mtx;
    struct my_server_traits : public restinio::default_single_thread_traits_t {
        using request_handler_t = restinio::router::express_router_t<>;
    };
    restinio::running_server_handle_t<my_server_traits> _server;

    // Describe the general status of the app
    enum app_status_code_t { ERROR = -1, LOADING, SUCCESS, UNDEFINED };

    struct app_status_t {
        app_status_code_t code;
        std::string description;
        std::string error;
    };

    app_status_t _appStatus = {app_status_code_t::UNDEFINED, "Initializing", ""};
    std::unique_ptr<restinio::router::express_router_t<>> _createRouter();

public:
    void stateCallback(app_status_code_t new_state, std::string_view description, std::string_view error);
    void run();
};
