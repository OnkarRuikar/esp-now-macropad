#!/usr/bin/env bash

cd /home/username/scripts/macro-server

# use the USB port the dongle is connected to
node index.js /dev/ttyUSB0
