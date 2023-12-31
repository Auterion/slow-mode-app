#include "app.h"

std::unique_ptr<restinio::router::express_router_t<>> App::_createRouter()
{
    auto router = std::make_unique<restinio::router::express_router_t<>>();
    router->http_get(R"(/status)", [this](auto req, [[maybe_unused]] auto params) {
        app_status_t appStatus;
        {
            // copy current state
            std::lock_guard<std::mutex> lock(_state_mtx);
            appStatus = _appStatus;
        }
        return req->create_response()
            .append_header(restinio::http_field::content_type, "text/json; charset=utf-8")
            .append_header(restinio::http_field::access_control_allow_origin, "*")
            .set_body(fmt::format(
                R"({{"status": {}, "status_description": "{}", "error": "{}"}})", appStatus.code, appStatus.description, appStatus.error))
            .done();
    });
    return router;
}

void App::stateCallback(app_status_code_t new_state, std::string_view description, std::string_view error)
{
    std::lock_guard<std::mutex> lock(_state_mtx);
    if (new_state == _appStatus.code && description == _appStatus.description && error == _appStatus.error) {
        return;
    }
    _appStatus.code = new_state;
    _appStatus.description = description;
    _appStatus.error = error;

    if (new_state == app_status_code_t::ERROR) {
        SPDLOG_ERROR("Slowmode App Error: {}", description);
    } else {
        SPDLOG_INFO("Slowmode App: {}", description);
    }
}

void App::run()
{
    // start server
    SPDLOG_INFO("Starting server on port 8080");
    _server = restinio::run_async<my_server_traits>(
        restinio::own_io_context(),
        restinio::server_settings_t<my_server_traits>{}.address("0.0.0.0").port(8080).request_handler(App::_createRouter()),
        1);
}
