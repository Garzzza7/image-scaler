#!/bin/bash

if [ -f "/etc/arch-release" ]; then
    echo "Arch detected!!!!!!"
    sudo pacman -S cmake make gcc
elif [ -f "/etc/debian_version" ]; then
    echo "Debian detected!!!!!!"
    sudo apt-get update
    sudo apt-get install g++ cmake make
fi
