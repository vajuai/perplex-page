# Perplex Page (Lavandula + Yew)

This project exposes a tiny HTTP API in C (Lavandula) that shells out to `simpcurl` to query Perplexity, and a Yew (Rust/WASM) frontend (using function_component) that displays those results in the browser.

## Prerequisites
- Backend build requirements: gcc, make, sqlite3 dev libs (for Lavandula), libcurl (for simpcurl)
- Frontend requirements: Rust toolchain, wasm32-unknown-unknown target, and Trunk
- Perplexity API key set in environment: `PERPLEXITY_API_KEY`

Install Rust + tools:
```bash
rustup target add wasm32-unknown-unknown
cargo install trunk
```

## Build backend
```bash
make
```
This produces the server at `./build/a`. You also need the `simpcurl` helper (binary) in the project root. If not present, build it:
```bash
gcc simpcurl.c -lcurl -o simpcurl
```

## Run backend
Set your API key and run the server:
```bash
export PERPLEXITY_API_KEY=your_api_key_here
./build/a
```

The backend provides:
- GET http://127.0.0.1:3000/api/search?q=your+query — runs `./simpcurl` and returns plain text results.

CORS is enabled by default for development so the Yew app can call this API from another origin.

## Run frontend (Yew)
Open a new terminal and start the Yew dev server:
```bash
cd frontend
trunk serve --open
```
This launches the frontend on http://127.0.0.1:8080 which talks to the backend at http://127.0.0.1:3000.

The Yew app (implemented with `#[function_component(App)]`) shows a search box, performs a fetch to `/api/search`, and renders the results.

## Optional: production build of the frontend
```bash
cd frontend
trunk build --release
```
This creates a `dist/` directory under `frontend/`. Serving the built assets from the C server would require a small static file handler (including `application/wasm` content type); currently we recommend running the dev server during development.

## Notes
- If `PERPLEXITY_API_KEY` is not set on the backend, the API will return an error and the frontend will show a message.
- The default query is "latest technology news". You can change it in `frontend/src/main.rs` and in the backend fallback in `app/controllers/search.c`.

## Publish to GitHub
You can upload this project to your own GitHub repository using either the GitHub CLI (recommended) or an existing remote URL.

Option A — Create a new repo via GitHub CLI
1) Install and authenticate:
```bash
gh auth login
```
2) From the project root, run:
```bash
chmod +x scripts/publish_to_github.sh
export GITHUB_REPO="perplex-page"          # change to your preferred repo name
export GITHUB_OWNER="your-username"        # optional; omit to use your default account
export GITHUB_VISIBILITY="private"         # or "public"
./scripts/publish_to_github.sh
```
This will initialize git (if needed), create the GitHub repo via gh, set the remote, and push the main branch.

Option B — Push to an existing repository URL
If you already created a repository on GitHub, provide its remote URL:
```bash
chmod +x scripts/publish_to_github.sh
export GITHUB_REMOTE="https://github.com/OWNER/REPO.git"   # or git@github.com:OWNER/REPO.git
./scripts/publish_to_github.sh
```

Manual steps (if you prefer not to use the script)
```bash
git init -b main
git add -A
git commit -m "Initial commit: Perplex Page (Lavandula + Yew)"
git remote add origin https://github.com/OWNER/REPO.git
# or: git remote add origin git@github.com:OWNER/REPO.git
git push -u origin main
```

Notes
- The repository includes a .gitignore that excludes build artifacts (build/, simpcurl, frontend/target, frontend/dist, .DS_Store).
- The script is idempotent: it won’t re-create commits if nothing changed and will reuse an existing origin remote if present.
- If the push fails due to permissions, ensure your GitHub CLI session is authenticated (gh auth status) or that your PAT/SSH setup is correct.
