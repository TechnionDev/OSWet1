cmake_minimum_required(VERSION 3.10)
project(OSWet1)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(OSWet1
        Commands.cpp
        Commands.h
        Signals.cpp
        Signals.h
        main.cpp
        Jobs.h
        Jobs.cpp Exceptions.h Utils.cpp Utils.h SmallShell.cpp SmallShell.h)
