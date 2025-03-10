# ==============================================
# = Dockerfile for cross-compilation (RPi 4B+) = 
# ==============================================

FROM debian:bookworm

COPY first_use_prepare.sh /usr/local/bin/first_use_prepare.sh

RUN chmod +x /usr/local/bin/first_use_prepare.sh

RUN ./usr/local/bin/first_use_prepare.sh -i

WORKDIR /app
