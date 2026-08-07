# Stage 1: Build

FROM alpine:latest AS build

# build dependencies
RUN apk add --no-cache \
    build-base \
    llvm22 \
    llvm22-dev \
    llvm22-static \
    llvm22-gtest \
    clang22 \
    clang22-dev \
    clang22-static \
    cmake \
    ninja \
    libxml2-dev \
    curl-dev \
    libxml2 \
    curl-dev

# copy the application sources
WORKDIR /app
COPY ./ ./

# configure the application
RUN cmake --preset release && \
    cmake --build --preset release --parallel

# Stage 2: Package

FROM alpine:latest AS runtime

# arguments
ARG DATE="unknown"
ARG VERSION="unknown"
ARG REVISION="unknown"

# labels
LABEL org.opencontainers.image.title="Clang Architecture Generator" \
      org.opencontainers.image.description="A C/C++ program architecture generator" \
      org.opencontainers.image.version="${VERSION}" \
      org.opencontainers.image.authors="Faraphel <faraphel@faraphel.fr>" \
      org.opencontainers.image.licenses="CC ANSA 4.0" \
      org.opencontainers.image.source="https://git.faraphel.fr/faraphel/clang-architecture-generator" \
      org.opencontainers.image.revision="${REVISION}" \
      org.opencontainers.image.created="${DATE}"

# runtime dependencies
RUN apk add --no-cache \
    llvm22-libs \
    clang22-libs

# copy the application binaries
WORKDIR /app
COPY --from=build /app/build/release/clang-architecture ./clang-architecture

# command
ENTRYPOINT ["./clang-architecture"]
CMD ["--help"]
