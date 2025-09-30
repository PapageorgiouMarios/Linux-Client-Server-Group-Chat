#!/bin/bash

echo "=== Server Test ==="

# Build and run the Docker container
echo "✓ Building and running Docker container"
cd docker
docker-compose run --rm server-test ./build/test_server
cd ..

echo "=== ✓ Server Test completed! ===" 