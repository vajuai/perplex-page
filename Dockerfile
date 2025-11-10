FROM debian:bookworm-slim

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        curl \
        ca-certificates \
        libcurl4-openssl-dev \
        libsqlite3-dev \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make && gcc simpcurl.c -lcurl -o simpcurl

ENV PORT=3000
EXPOSE 3000

CMD ["./build/a"]
