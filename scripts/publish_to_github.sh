#!/usr/bin/env bash
set -euo pipefail

# Publish this project to GitHub.
#
# Usage (new repo via GitHub CLI):
#   gh auth login
#   export GITHUB_REPO="perplex-page"        # required
#   export GITHUB_OWNER="your-username"      # optional (default: your gh default)
#   export GITHUB_VISIBILITY="private"       # optional: public|private (default: private)
#   bash scripts/publish_to_github.sh
#
# Usage (existing remote):
#   export GITHUB_REMOTE="https://github.com/owner/repo.git"
#   bash scripts/publish_to_github.sh
#
# Notes:
# - This script initializes git (if needed), creates an initial commit, sets origin,
#   and pushes the main branch. If GitHub CLI (gh) is available and configured and
#   no GITHUB_REMOTE is provided, it will create the repository for you.

log() { printf "\033[1;34m[info]\033[0m %s\n" "$*"; }
warn() { printf "\033[1;33m[warn]\033[0m %s\n" "$*"; }
err()  { printf "\033[1;31m[error]\033[0m %s\n" "$*"; }

ensure_git_main() {
  if [ ! -d .git ]; then
    log "Initializing git repository"
    if git init -b main >/dev/null 2>&1; then
      :
    else
      git init >/dev/null
      git checkout -b main >/dev/null 2>&1 || true
    fi
  else
    # Ensure we are on main (create if needed)
    if ! git rev-parse --abbrev-ref HEAD >/dev/null 2>&1; then
      warn "Unable to detect current branch; continuing"
    else
      current_branch=$(git rev-parse --abbrev-ref HEAD)
      if [ "$current_branch" != "main" ]; then
        if git show-ref --verify --quiet refs/heads/main; then
          git checkout main >/dev/null 2>&1 || true
        else
          git checkout -b main >/dev/null 2>&1 || true
        fi
      fi
    fi
  fi
}

make_initial_commit() {
  # Respect existing .gitignore
  git add -A
  if git diff --cached --quiet; then
    log "No changes to commit (repository already committed)."
  else
    log "Creating initial commit"
    git commit -m "Initial commit: Perplex Page (Lavandula + Yew)" >/dev/null
  fi
}

setup_remote_and_push() {
  local remote_url="$1"
  if git remote get-url origin >/dev/null 2>&1; then
    log "Using existing remote: $(git remote get-url origin)"
  else
    log "Adding origin remote: $remote_url"
    git remote add origin "$remote_url"
  fi

  log "Pushing main -> origin"
  git push -u origin main
}

create_with_gh_and_push() {
  local repo="$1"         # name or owner/name
  local visibility="$2"   # public|private

  if ! command -v gh >/dev/null 2>&1; then
    err "GitHub CLI (gh) not found. Install from https://cli.github.com/ or set GITHUB_REMOTE."
    exit 1
  fi

  # Create repo via gh and push current source.
  # --source=. uses the current directory; requires a commit first.
  local vis_flag="--private"
  if [ "$visibility" = "public" ]; then
    vis_flag="--public"
  fi

  log "Creating GitHub repository '$repo' ($visibility) via gh"
  gh repo create "$repo" $vis_flag --source=. --remote=origin --push
}

main() {
  # Sanity check: run from project root (expect README.md and makefile)
  if [ ! -f README.md ] || [ ! -f makefile ]; then
    warn "Not running from project root; continuing anyway"
  fi

  ensure_git_main
  make_initial_commit

  local remote="${GITHUB_REMOTE:-}"
  if [ -n "$remote" ]; then
    setup_remote_and_push "$remote"
    log "Done. Repository pushed to: $remote"
    exit 0
  fi

  local repo_name="${GITHUB_REPO:-}"
  local owner="${GITHUB_OWNER:-}"
  local visibility="${GITHUB_VISIBILITY:-private}"

  if [ -z "$repo_name" ]; then
    err "GITHUB_REPO is not set and GITHUB_REMOTE not provided.\nSet GITHUB_REPO (and optionally GITHUB_OWNER, GITHUB_VISIBILITY) or set GITHUB_REMOTE."
    exit 1
  fi

  local full="$repo_name"
  if [ -n "$owner" ]; then
    full="$owner/$repo_name"
  fi

  create_with_gh_and_push "$full" "$visibility"
  log "Done. Repository created and pushed: https://github.com/$full"
}

main "$@"
