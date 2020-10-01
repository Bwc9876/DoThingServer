# CMakeList.txt : CMake project for DoThingServer, include source and define
# project specific logic here.
#

cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project ("DoThingServer")

set(SOURCES DoThingServer.cpp DoThingServer.h)

# Add source to this project's executable.
add_executable (DoThingServer "DoThingServer.cpp" "DoThingServer.h" "Connection.cpp" "Connection.h")

# TODO: Add tests and install targets if needed.
