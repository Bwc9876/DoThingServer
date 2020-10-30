#!/bin/bash

echo "Compiling..."

g++ -std=c++17 -g3 -ggdb -O0 -Wall -Wextra -Wno-unused -o DoThingServer Connection.cpp DoThingServer.cpp -lcryptopp

echo "Compilation Finished"