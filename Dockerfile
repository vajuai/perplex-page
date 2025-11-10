FROM rust:1.81-slim AS builder

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        pkg-config \
        libcurl4-openssl-dev \
        libsqlite3-dev \
        ca-certificates \
        wget \
    && rm -rf /var/lib/apt/lists/*

RUN rustup target add wasm32-unknown-unknown \
    && cargo install trunk

WORKDIR /src

COPY . .

RUN make \
    && gcc simpcurl.c -lcurl -o simpcurl \
    && cd frontend && trunk build --release

FROM debian:bookworm-slim AS runtime

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libcurl4 \
        libsqlite3-0 \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /src/build/a ./build/a
COPY --from=builder /src/simpcurl ./simpcurl
COPY --from=builder /src/frontend/dist ./frontend/dist
COPY --from=builder /src/lavandula.yml ./lavandula.yml

RUN chmod +x ./build/a ./simpcurl

ENV PORT=3000
EXPOSE 3000

CMD ["./build/a"]
