#!/bin/bash

PLATFORM="linux/arm/v7"
IMAGE_NAME="rpi4b-cross-compile"
DOCKERFILE_DIR="./tools/cross-compilation"

build_image() {
    echo "Building Docker image..."
    docker buildx build --platform $PLATFORM -t $IMAGE_NAME $DOCKERFILE_DIR
    if [ $? -eq 0 ]; then
        echo "Image built successfully."
    else
        echo "Error building the image."
        exit 1
    fi
}

run_container() {
    echo "Starting the container..."
    docker run -it --rm --platform $PLATFORM -v $(pwd):/app $IMAGE_NAME /bin/bash
    if [ $? -ne 0 ]; then
        echo "Error starting the container."
        exit 1
    fi
}

case "$1" in
    "build")
        build_image
        ;;
    "run")
        run_container
        ;;
    "brun")
        build_image
        run_container
        ;;
    *)
        echo "Usage: $0 [build|run|brun]"
        echo "  build       - Build Docker image."
        echo "  run         - Run the container with project folder mounted."
        echo "  brun        - Build image and run the container."
        exit 1
        ;;
esac
