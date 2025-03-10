#!/bin/bash
set -e

# ========
# = Init =
# ========

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 1
fi

INSTALL_ONLY=false
while getopts ":i" opt; do
    case ${opt} in
    i)
        INSTALL_ONLY=true
        ;;
    \?)
        echo "Invalid option: -$OPTARG" >&2
        exit 1
        ;;
    esac
done

BASE_DIR=$(dirname $(dirname $(realpath $0)))
MODELS_DIR="$BASE_DIR/models"
TOOLS_DIR="$BASE_DIR/tools"

mkdir -p "$MODELS_DIR"

# =============================== COMMON PART ==================================

# ===============================
# = General components and libs =
# ===============================

apt-get update && apt-get install -y \
    build-essential \
    make \
    wget \
    unzip \
    libgpiod-dev \
    libasound2-dev \
    libcjson-dev \
    libcurl4-openssl-dev

# ========
# = VOSK =
# ========

wget https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-armv7l-0.3.45.zip -P "$TOOLS_DIR" && \
    unzip -o "$TOOLS_DIR/vosk-linux-armv7l-0.3.45.zip" -d "$TOOLS_DIR" && \
    cp "$TOOLS_DIR/vosk-linux-armv7l-0.3.45/libvosk.so" /lib/ && \
    cp "$TOOLS_DIR/vosk-linux-armv7l-0.3.45/vosk_api.h" /usr/local/include/ && \
    rm -rf "$TOOLS_DIR/vosk-linux-armv7l-0.3.45" "$TOOLS_DIR/vosk-linux-armv7l-0.3.45.zip"

wget https://alphacephei.com/vosk/models/vosk-model-small-en-us-0.15.zip -P "$MODELS_DIR" && \
    unzip -o "$MODELS_DIR/vosk-model-small-en-us-0.15.zip" -d "$MODELS_DIR" && \
    rm "$MODELS_DIR/vosk-model-small-en-us-0.15.zip"

# ================================= RPi PART ===================================

if [ "$INSTALL_ONLY" = true ]; then
    echo "Installation complete. No system modifications were made."
    exit 0
fi

# ================
# = Raspi-config =
# ================

apt-get update && apt-get install -y \
    raspi-config

raspi-config nonint do_spi 0
raspi-config nonint do_i2c 0
raspi-config nonint do_i2s 0

# =============
# = Mic setup =
# =============

BOOT_CONFIG="/boot/firmware/config.txt"
if [ ! -f "$BOOT_CONFIG" ]; then
    BOOT_CONFIG="/boot/config.txt"
    if [ ! -f "$BOOT_CONFIG" ]; then
        echo "Can't find config.txt"
        exit 1
    fi
fi

echo "dtoverlay=googlevoicehat-soundcard" | tee -a "$BOOT_CONFIG" > /dev/null

# ==========
# = Ollama =
# ==========

curl -fsSL https://ollama.ai/install.sh | sh

ollama pull deepseek
ollama serve &

# ===================
# = Systemd service =
# ===================

systemctl enable "$TOOLS_DIR/service/piTalkster.service"

# ======================
# = Finish with reboot =
# ======================

read -p "Installation complete. A reboot is required. Reboot now? (y/n): " REBOOT
if [[ "$REBOOT" =~ ^[Yy]$ ]]; then
    reboot
fi
