#!/bin/bash
if grep -q PLATFORM_NS .vscode/c_cpp_properties.json
then
    echo "Compiling for PLATFORM_NS"
    make ns
elif grep -q PLATFORM_WINDOWS .vscode/c_cpp_properties.json
then
    echo "Compiling for PLATFORM_WINDOWS"
    make
elif grep -q PLATFORM_LINUX .vscode/c_cpp_properties.json
then
    echo "Compiling for PLATFORM_LINUX"
    make
else
    # code if not found
    echo "Platform not found in .vscode/c_cpp_properties.json, aborting..."
fi