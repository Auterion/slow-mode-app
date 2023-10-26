cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(slowmode-app
        VERSION 0.1.0
        DESCRIPTION "An application that limits the velocities setpoint of the PX4 autopilot"
        LANGUAGES CXX)

message("-- CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("-- CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

find_package(unofficial-http-parser REQUIRED)

find_package(fmt REQUIRED)
if (fmt_FOUND)
	message(STATUS "Found fmt: ${fmt_VERSION}")
endif()

find_package(restinio CONFIG REQUIRED)
if (restinio_FOUND)
	message(STATUS "Found restinio: ${restinio_VERSION}")
endif()

find_package(spdlog CONFIG REQUIRED)
if (spdlog_FOUND)
	message(STATUS "Found spdlog: ${spdlog_VERSION}")
endif()

add_executable(${PROJECT_NAME}-udp main.cpp VelocityLimits.cpp ConnectionHandler.cpp app.cpp)
target_compile_definitions(${PROJECT_NAME}-udp PRIVATE UDP)
target_include_directories(${PROJECT_NAME}-udp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/libmav/include)
target_link_libraries(${PROJECT_NAME}-udp spdlog::spdlog unofficial::http_parser::http_parser)

add_executable(${PROJECT_NAME}-tcp main.cpp VelocityLimits.cpp ConnectionHandler.cpp app.cpp)
target_include_directories(${PROJECT_NAME}-tcp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/libmav/include)
target_link_libraries(${PROJECT_NAME}-tcp spdlog::spdlog unofficial::http_parser::http_parser)

# copy mavlink message definitions to the build directory
file(GLOB MAVLINK_XML ${CMAKE_CURRENT_SOURCE_DIR}/libs/mavlink/message_definitions/v1.0/*.xml)
file(COPY ${MAVLINK_XML} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/mavlink)
