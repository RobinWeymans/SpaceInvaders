cmake_minimum_required(VERSION 3.1)
project(Emulator)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)
find_package(Threads REQUIRED)
find_package(X11 REQUIRED)

add_executable(Emulator src/main.cpp src/Machine8080.cpp src/Machine8080.h src/State8080.cpp src/State8080.h src/disassemble.cpp src/emulate.cpp)

target_link_libraries(Emulator sfml-graphics sfml-audio)
target_link_libraries(Emulator Threads::Threads)