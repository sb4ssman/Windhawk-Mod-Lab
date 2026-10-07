#!/usr/bin/env bash
# Bring THIS machine's clone of the lab in line with origin/main.
# Safe to run at the start of any session, on any machine (Windows Git Bash,
# Linux, macOS):   bash .agents/tools/sync-lab.sh
#
# It never discards work. It fast-forwards when it can, and STOPS and tells you
# why when it cannot. The one case it fixes by itself is a clone that still has
# the three commits the Oct 7 2026 history scrub removed (see the list below).
#
# Background: the merge commit e12209f added raw tree dumps containing personal
# window titles to the public repo. History was rewritten and force-pushed
# (origin/main became bac61b3). Commits before e12209f kept their SHAs, so most
# clones only need a plain fast-forward.

set -u
cd "$(git rev-parse --show-toplevel)" || exit 1

# The old commits that no longer exist on origin. A clone holding ONLY these
# as local-only commits has nothing of its own to lose.
OLD_COMMITS="e12209f b8ff6b1 2a61972"

say() { printf '%s\n' "$*"; }

git fetch origin || { say "STOP: cannot reach origin."; exit 1; }

branch=$(git rev-parse --abbrev-ref HEAD)
if [ "$branch" != "main" ]; then
    say "STOP: you are on '$branch', not main. Switch to main first (or finish that branch)."
    exit 1
fi

if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
    say "STOP: you have uncommitted changes to tracked files. Commit or stash them first:"
    git status --short --untracked-files=no
    exit 1
fi

local_sha=$(git rev-parse HEAD)
remote_sha=$(git rev-parse origin/main)

if [ "$local_sha" = "$remote_sha" ]; then
    say "OK: already up to date with origin/main ($(git rev-parse --short HEAD))."
    exit 0
fi

if git merge-base --is-ancestor HEAD origin/main; then
    git merge --ff-only origin/main >/dev/null && \
        say "OK: fast-forwarded to origin/main ($(git rev-parse --short HEAD))."
    exit $?
fi

if git merge-base --is-ancestor origin/main HEAD; then
    say "NOTE: you are AHEAD of origin/main with commits that are not pushed:"
    git log --oneline origin/main..HEAD
    say "Review them, then 'git push origin main' (a normal push, never --force)."
    exit 0
fi

# Diverged. Is every local-only commit one of the scrubbed ones?
local_only=$(git rev-list origin/main..HEAD)
foreign=""
for sha in $local_only; do
    short=$(git rev-parse --short=7 "$sha")
    case " $OLD_COMMITS " in
        *" $short "*) ;;
        *) foreign="$foreign $short" ;;
    esac
done

if [ -n "$foreign" ]; then
    say "STOP: your clone has commits of its own that origin/main does not have:"
    git log --oneline origin/main..HEAD
    say ""
    say "Do not reset. Hand this output to the user or an LLM to rebase or"
    say "cherry-pick them onto origin/main, then push normally."
    exit 1
fi

say "This clone holds only the old (scrubbed) commits: $OLD_COMMITS."
say "Nothing of yours is lost - origin/main has the same work minus the dumps."
git reset --hard origin/main && say "OK: reset to origin/main ($(git rev-parse --short HEAD))."
say "Optional tidy-up, to drop the old commits from this disk:"
say "  git reflog expire --expire=now --all && git gc --prune=now"
