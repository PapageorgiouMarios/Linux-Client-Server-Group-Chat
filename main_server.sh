#!/bin/bash

echo "=== Server Main ==="
PORT=${1:-8080}

# Build and run the Docker container
echo "✓ Building and running Docker container"
cd docker
docker-compose build main-server
docker-compose run --rm main-server "$PORT"
cd ..

echo "=== ✓ Server Main completed! ===" 