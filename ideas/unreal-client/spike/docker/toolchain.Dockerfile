# Toolchain for the spike's headless client: clang + libc++ (TurboLink's Linux
# libs are libc++ builds) + what those libs reference: OpenSSL, zlib, c-ares.
FROM ubuntu:24.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      clang-18 libc++-18-dev libc++abi-18-dev libssl-dev zlib1g-dev libc-ares-dev \
      binutils ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    && ln -s /usr/bin/clang++-18 /usr/local/bin/clang++
