FROM ubuntu:22.04 AS builder 

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake git ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY .. /app

RUN mkdir -p build && cd build \
    && cmake .. \
    && cmake --build . -- -j$(nproc)

RUN chmod +x /app/build/test_server /app/build/test_client /app/build/main_server /app/build/main_client

# ===== Test Server Target =====
FROM ubuntu:22.04 AS server
WORKDIR /app
COPY --from=builder /app/build/test_server /app/build/test_server
ENTRYPOINT ["/app/build/test_server"]

# ===== Test Client Target =====
FROM ubuntu:22.04 AS client
WORKDIR /app
COPY --from=builder /app/build/test_client /app/build/test_client
ENTRYPOINT ["/app/build/test_client"]

# ===== Server Target =====
FROM ubuntu:22.04 AS main_server
WORKDIR /app
COPY --from=builder /app/build/main_server /app/build/main_server
ENTRYPOINT ["/app/build/main_server"]
CMD ["8080"]

# ===== Client Target =====
FROM ubuntu:22.04 AS main_client
WORKDIR /app
COPY --from=builder /app/build/main_client /app/build/main_client
ENTRYPOINT ["/app/build/main_client"]
CMD ["8080"]

