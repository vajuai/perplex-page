FROM debian:bookworm-slim AS builder

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        pkg-config \
        libcurl4-openssl-dev \
        libsqlite3-dev \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY . .

RUN make \
    && gcc simpcurl.c -lcurl -o simpcurl

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
COPY --from=builder /src/frontend_htmx ./frontend_htmx
COPY --from=builder /src/lavandula/build/liblavandula.a ./lib/liblavandula.a
COPY lavandula.yml ./lavandula.yml

RUN chmod +x ./build/a ./simpcurl

ENV PORT=3000
EXPOSE 3000

CMD ["./build/a"]
