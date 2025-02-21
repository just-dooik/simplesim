# Use Ubuntu as the base image
FROM ubuntu:20.04

# Set environment to non-interactive
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages, including flex
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    gcc \
    g++ \
    wget \
    tar \
    flex \
    bison \
    vim \
    git && \
    apt-get clean

# Set the working directory
WORKDIR /simplesim-3.0

# Copy the SimpleScalar files
COPY simplesim-3.0/ /simplesim-3.0/

# Build the source code
RUN make clean && make

# Default command
CMD ["/bin/bash"]