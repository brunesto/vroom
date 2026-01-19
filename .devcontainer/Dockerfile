# Use Debian 12 (Bookworm) as the base image
FROM gcc:13

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools 
RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    bear \
    git \
    pkgconf \
    libglpk-dev \
    glpk-utils \ 
    libasio-dev \
    python3 \
    clangd \
    && apt-get clean -y && rm -rf /var/lib/apt/lists/*
    
# frontend to default
ENV DEBIAN_FRONTEND=dialog

