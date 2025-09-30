#!/bin/bash

echo "=== Client Main ==="
HOST=${1:-main-server}
PORT=${2:-8080}

echo "✓ Building and running Docker container"
cd docker
docker-compose build main-client
docker-compose run --rm -it main-client "$HOST" "$PORT"
cd ..

echo "=== ✓ Client Main completed! ==="
