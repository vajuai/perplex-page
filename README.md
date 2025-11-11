# Perplex Page (Lavandula + HTMX)

This project is a thin Lavandula (C) backend that shells out to `simpcurl` for Perplexity queries and serves a lightweight HTMX frontend. Everything now builds against the upstream Lavandula repo pulled in as a git submodule.

## Prerequisites
- Build toolchain: `gcc`, `make`, `pkg-config`
- Libraries: `libcurl` (for `simpcurl`), `libsqlite3`
- Perplexity API key exported as `PERPLEXITY_API_KEY`

## Pulling the repo
```bash
git clone https://github.com/vajuai/perplex-page.git
cd perplex-page
git checkout lib-thin-app
git submodule update --init --recursive
```

## Build & run locally
```bash
make                 # builds lavandula/liblavandula.a, build/a, simpcurl
export PERPLEXITY_API_KEY=your_key
./build/a            # backend + HTMX frontend at http://127.0.0.1:3000/
```

Endpoints:
- `GET /` or `/htmx` – HTMX page
- `GET /htmx/search?q=...` – returns HTML snippet
- `GET /api/search?q=...` – plain text output from Perplexity

## Deploying to Fly.io
```bash
flyctl deploy --remote-only
```
The Docker build compiles Lavandula and your thin app inside Debian. Runtime image includes only `build/a`, `simpcurl`, `frontend_htmx/`, and `lib/liblavandula.a`.

## Keeping Lavandula up to date
Lavandula lives as a submodule (`https://github.com/ashtonjamesd/lavandula.git`). To refresh:
```bash
git submodule update --remote lavandula
# optional: checkout a specific tag
cd lavandula && git checkout <tag-or-commit> && cd ..
git add lavandula .gitmodules
git commit -m "chore: bump lavandula to <id>"
```
Rebuild (`make`) and redeploy afterwards.

## Notes
- Only the HTMX frontend ships now; the old Yew/WASM setup has been removed to keep the image minimal.
- `make clean` wipes `build/` and `simpcurl`; the Lavandula build directory is regenerated automatically.
- All Lavandula headers live under `lavandula/src/include`.
- If you fork Lavandula, update the submodule URL accordingly.
