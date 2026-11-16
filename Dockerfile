# ---------- Build stage ----------
FROM debian:bookworm AS build

ARG REDIS_PLUS_PLUS_VERSION=1.3.11
ARG CROW_VERSION=v1.2.0

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates g++ cmake make git libasio-dev libhiredis-dev libssl-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /deps
RUN git clone --depth 1 --branch ${REDIS_PLUS_PLUS_VERSION} https://github.com/sewenew/redis-plus-plus.git && \
    cmake -S redis-plus-plus -B redis-plus-plus/build -DCMAKE_BUILD_TYPE=Release \
        -DREDIS_PLUS_PLUS_BUILD_TEST=OFF -DREDIS_PLUS_PLUS_BUILD_SHARED=OFF && \
    cmake --build redis-plus-plus/build -j"$(nproc)" && cmake --install redis-plus-plus/build

RUN git clone --depth 1 --branch ${CROW_VERSION} https://github.com/CrowCpp/Crow.git && \
    cmake -S Crow -B Crow/build -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF && \
    cmake --install Crow/build

WORKDIR /src
COPY compile.sh main.cpp ./
COPY src ./src
RUN ./compile.sh

# ---------- Runtime stage ----------
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends libhiredis0.14 libssl3 && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /usr/src/app
COPY --from=build /src/app ./app

EXPOSE 8003
CMD ["./app"]
