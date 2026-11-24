# Runs TurboLink's prebuilt generator (Tools/protoc-gen-turbolink.exe, a .NET
# Framework 4.6.2 program) under mono, driven by protoc, over our protos.
FROM debian:bookworm-slim
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      mono-devel protobuf-compiler libprotobuf-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*
