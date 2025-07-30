#!/bin/bash

BUILD_DIR="release"
SRC_BASE="server"
BUILD_BASE="$BUILD_DIR/server"
SERVER_BUILD="$BUILD_BASE/dpgs-server"
DEV_BUILD="$BUILD_BASE/dpgs-dev"

function run_server {
    cd "$SERVER_BUILD" || {
        echo "[ERROR] Failed to enter server directory"
        exit 1
    }

    ./dpgs-server
}

function run_drivers {
    KMOD="$DEV_BUILD/drivers/rgbmatrix.ko"
    RECV_BIN="$DEV_BUILD/utility/recv"
    DEVICE="/dev/rgbmatrix"
    MAJOR=260
    MINOR=0

    echo "[INFO] Inserting kernel module: $KMOD"
    sudo insmod "$KMOD" || {
        echo "[ERROR] insmod failed"
        exit 1
    }

    echo "[INFO] Creating device node: $DEVICE (major=$MAJOR, minor=$MINOR)"
    sudo mknod "$DEVICE" c $MAJOR $MINOR || {
        echo "[ERROR] Failed to create device node"
        sudo rmmod rgbmatrix
        exit 1
    }

    echo "[INFO] Setting permission to 666 for $DEVICE"
    sudo chmod 666 "$DEVICE"

    echo "[INFO] Running user-space binary: $RECV_BIN"
    "$RECV_BIN"

    echo "[INFO] Cleaning up..."
    echo "[INFO] Removing device node: $DEVICE"
    sudo rm -f "$DEVICE"

    echo "[INFO] Removing kernel module: rgbmatrix"
    sudo rmmod rgbmatrix
}

# === Main entry point ===
case "$1" in
    server)
        run_server
        ;;
    drivers)
        run_drivers
        ;;
    *)
        echo "Usage: $0 {server|drivers}"
        exit 1
        ;;
esac
