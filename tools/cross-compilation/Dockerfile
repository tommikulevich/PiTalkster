# ==============================================
# = Dockerfile for cross-compilation (RPi 4B+) = 
# ==============================================

FROM debian:bookworm

COPY initial_setup.sh /usr/local/bin/initial_setup.sh

RUN chmod +x /usr/local/bin/initial_setup.sh

RUN ./usr/local/bin/initial_setup.sh -i

WORKDIR /app
