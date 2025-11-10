FROM debian:bookworm-slim

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libcurl4 \
        libsqlite3-0 \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY build/a ./build/a
COPY simpcurl ./simpcurl
COPY frontend/dist ./frontend/dist
COPY lavandula.yml ./lavandula.yml

RUN chmod +x ./build/a ./simpcurl

ENV PORT=3000
EXPOSE 3000

CMD ["./build/a"]
