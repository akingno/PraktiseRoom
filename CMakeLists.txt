cmake_minimum_required(VERSION 3.27)
project(room_temp)

set(CMAKE_CXX_STANDARD 17)

add_executable(room_temp main.cpp
        Room.h
        Room.cpp
        Random.cpp
        AVector2d.h
        Utils.h)
