# ==============================================
# = Dockerfile for cross-compilation (RPi 4B+) = 
# ==============================================

FROM arm32v7/debian:bookworm

RUN apt-get update && apt-get install -y \
    build-essential make wget unzip \
    libgpiod-dev \
    libasound2-dev \
    libcjson-dev \
    libcurl4-openssl-dev && \
    rm -rf /var/lib/apt/lists/*

RUN wget https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-armv7l-0.3.45.zip && \
    unzip vosk-linux-armv7l-0.3.45.zip && \
    cp vosk-linux-armv7l-0.3.45/libvosk.so /lib/ && \
    cp vosk-linux-armv7l-0.3.45/vosk_api.h /usr/local/include/ && \
    rm -rf vosk-linux-armv7l-0.3.45 vosk-linux-armv7l-0.3.45.zip

WORKDIR /app
