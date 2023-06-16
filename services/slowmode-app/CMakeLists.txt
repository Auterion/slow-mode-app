cmake_minimum_required(VERSION 3.16)

project(slowmode
        VERSION 0.1.0
        DESCRIPTION "An application that limits the velocities setpoint of the PX4 autopilot"
        LANGUAGES CXX)

add_executable(slowmode main.cpp)