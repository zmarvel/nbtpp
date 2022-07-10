FROM debian:bullseye

RUN apt update && \
    apt install -y --no-install-recommends make cmake ninja-build clang g++ xxd libasan5 catch2 && \
    rm -rf /var/lib/apt/lists/*
