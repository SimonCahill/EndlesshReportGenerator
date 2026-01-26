FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    lcov \
    git \
    ca-certificates \
    pkg-config \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Copy project into image
COPY . /src

# Prepare build (do not run coverage at build time)
RUN cmake -S . -B build -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build -j

ENTRYPOINT ["/src/scripts/coverage.sh"]
