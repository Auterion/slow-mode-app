cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(slowmode-app
        VERSION 0.1.0
        DESCRIPTION "An application that limits the velocities setpoint of the PX4 autopilot"
        LANGUAGES CXX)

message("-----CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("-----CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

# Add your main application
add_executable(slowmode-app main.cpp VelocityLimits.cpp)
target_include_directories(slowmode-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/libmav/include)

# copy mavlink message definitions to the build directory
file(GLOB MAVLINK_XML ${CMAKE_CURRENT_SOURCE_DIR}/libs/mavlink/message_definitions/v1.0/*.xml)
file(COPY ${MAVLINK_XML} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/mavlink)
