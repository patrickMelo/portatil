#!/bin/sh

#
# Utils/SerialMonitor.sh
#
# This file is part of Portatil source code.
# Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
#

DEVICE_PATH=/dev/ttyACM0
BAUD_RATE=115200

runMonitor() {
    echo ""
    echo "Waiting for device to become available..."
    echo ""

    while ! [ -e "$DEVICE_PATH" ];
    do
        sleep 1
    done

    stty -F "$DEVICE_PATH" raw $BAUD_RATE
    cat "$DEVICE_PATH"
    runMonitor
}

runMonitor