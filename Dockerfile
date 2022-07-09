FROM debian:bullseye

RUN apt update && \
    apt install -y --no-install-recommends make g++ xxd libasan5 && \
    rm -rf /var/lib/apt/lists/*
