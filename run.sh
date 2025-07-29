#!/bin/bash

ORIG_DIR="$(pwd)"

function cleanup {
    echo "[INFO] Returning to original directory..."
    cd "$ORIG_DIR"
}
trap cleanup EXIT

cd build/server/dpgs-server || {
    echo "[ERROR] Failed to enter target directory"
    exit 1
}

./dpgs-server
