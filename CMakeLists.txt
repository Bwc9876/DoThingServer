# CMakeList.txt : CMake project for DoThingServer, include source and define
# project specific logic here.
#

cmake_minimum_required (VERSION 3.8)


project ("DoThingServer")

# Add source to this project's executable.
add_executable (DoThingServer "DoThingServer.cpp" "DoThingServer.h")

# TODO: Add tests and install targets if needed.
