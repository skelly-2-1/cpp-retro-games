#!/bin/bash
NS_IP=192.168.178.22 # IP of the Nintendo Switch in your local network

if grep -q PLATFORM_NS .vscode/c_cpp_properties.json
then
    # Send the .nro file to the Nintendo Switch
    echo "Sending bin/ns/cpp-retro-games.nro to NS"
    $DEVKITPRO/tools/bin/nxlink -s bin/ns/cpp-retro-games.nro -a $NS_IP
elif grep -q PLATFORM_WINDOWS .vscode/c_cpp_properties.json
then
    echo "Running bin/windows/cpp-retro-games.exe"
    cd bin/windows
    ./cpp-retro-games.exe
elif grep -q PLATFORM_LINUX .vscode/c_cpp_properties.json
then
    echo "Running bin/linux/cpp-retro-games"
    cd bin/linux
    ./cpp-retro-games
else
    # code if not found
    echo "Platform not found in .vscode/c_cpp_properties.json, aborting..."
fi