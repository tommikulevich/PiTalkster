#!/bin/bash

TARGET="rpi"

TOOLS_DIR=$(dirname "$(readlink -f "$0")")
PROJECT_DIR=$(dirname "$TOOLS_DIR")
BUILD_DIR="$PROJECT_DIR/build"
BIN_PATH="$BUILD_DIR/$TARGET/piTalkster"

if [ ! -f "$BIN_PATH" ]; then
  echo "Error: binary not found at $BIN_PATH"
  exit 1
fi

cd "$PROJECT_DIR" || { echo "Error: can't enter to $PROJECT_DIR"; exit 1; }

exec "$BIN_PATH"
