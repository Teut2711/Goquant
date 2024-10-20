# Use the official Ubuntu image
FROM debian:bookworm-slim

# Set the working directory
WORKDIR /trading

# Update package list and install necessary packages including g++, Boost, and OpenSSL
RUN apt-get update && \
    apt-get install -y \
    g++ \
    make \
    cmake \
    bash \
    libboost-all-dev \
    libssl-dev \
    libwebsocketpp-dev \
    libcrypto++-dev/stable \
    openssl \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*
EXPOSE 8080
# Command to keep the container running
CMD ["sleep", "infinity"]
