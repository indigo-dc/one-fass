#!/bin/sh
echo "Stopping fass..."
sudo systemctl stop fass
echo "Building..."
scons
echo "Installing..."
sudo ./install.sh
echo "Starting fass..."
sudo systemctl start fass
echo "And the result is..."
sudo systemctl status fass
