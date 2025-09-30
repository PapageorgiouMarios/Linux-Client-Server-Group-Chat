#!/bin/bash

echo "=== Client Test ==="

# Build and run the Docker container
echo "✓ Building and running Docker container"
cd docker
docker-compose run --rm client-test ./build/test_client
cd ..

echo "=== ✓ Client Test completed! ===" 