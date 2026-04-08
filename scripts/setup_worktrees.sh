#!/usr/bin/env bash
# Create per-agent git worktrees for parallel Claude Code sessions.

set -euo pipefail

PHASE="${1:-1}"
PARENT="$(cd .. && pwd)"
MAIN="$(basename "$PWD")"

ROLES=(architect backend frontend qa integration docs)

echo "Creating worktrees for phase ${PHASE} under ${PARENT}/"

for role in "${ROLES[@]}"; do
    DIR="${PARENT}/${MAIN}-${role}"
    BRANCH="agent/${role}/phase-${PHASE}"
    if [ -d "${DIR}" ]; then
        echo "  [skip] ${DIR} already exists"
        continue
    fi
    git worktree add "${DIR}" -b "${BRANCH}" main
    echo "  [ok]   ${DIR} on ${BRANCH}"
done

echo
echo "Done. Worktrees:"
git worktree list
