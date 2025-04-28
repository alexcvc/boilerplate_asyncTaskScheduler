FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    cmake \
    make \
    g++ \
    libspdlog-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir build && cd build && cmake .. && make

CMD ["./build/bin/main"]