#!/bin/bash

function build_server() {
    echo "[BUILD] Building server..."

    SRC_BASE="server"
    BUILD_BASE="build/server"

    SERVER_SRC="$SRC_BASE/dpgs-server"
    RGB_SRC="$SRC_BASE/dpgs-dev/rgbmatrix"
    SERVER_BUILD="$BUILD_BASE/dpgs-server"
    DEV_BUILD="$BUILD_BASE/dpgs-dev"

    make -C "$SERVER_SRC"
    make -C "$RGB_SRC"

    mkdir -p "$DEV_BUILD/drivers"
    mkdir -p "$DEV_BUILD/utility"
    mkdir -p "$SERVER_BUILD"

    cp "$RGB_SRC/rgbmatrix.ko" "$DEV_BUILD/drivers/"
    cp "$RGB_SRC/recv"         "$DEV_BUILD/utility/"
    cp "$SERVER_SRC/build/dpgs-server" "$SERVER_BUILD/"
    cp -r "$SERVER_SRC/config"         "$SERVER_BUILD/"

    sed -i "s/__YOUR_IP__/$(hostname -I | awk '{print $1}')/g" "$SERVER_BUILD/config/san.conf" 
    openssl req -x509 -nodes -newkey rsa:2048 \
    -keyout "$SERVER_BUILD/config/server.key" -out "$SERVER_BUILD/config/server.crt" \
    -days 365 -config "$SERVER_BUILD/config/san.conf" -extensions req_ext

    make -C "$SERVER_SRC" clean
    make -C "$RGB_SRC" clean
}


function build_client() {
    echo "[BUILD] Building client..."
    make -C client/dpgs-client
}

case "$1" in
    server)
        build_server
        ;;
    client)
        build_client
        ;;
    all)
        build_server
        build_client
        ;;
    clean)
        echo "[CLEAN] Removing build directory..."
        rm -rf build
        ;;
    *)
        echo "Usage: $0 {server|client|all}"
        exit 1
        ;;
esac
