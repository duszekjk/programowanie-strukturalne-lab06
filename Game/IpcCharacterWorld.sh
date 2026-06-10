#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

POSSIBLE_BINARIES=(
  "$SCRIPT_DIR/IpcCharacterWorld/Binaries/Linux/IpcCharacterWorld-Linux-Shipping"
  "$SCRIPT_DIR/IpcCharacterWorld/Binaries/Linux/IpcCharacterWorld-Linux-DebugGame"
  "$SCRIPT_DIR/Binaries/Linux/IpcCharacterWorld-Linux-Shipping"
  "$SCRIPT_DIR/Binaries/Linux/IpcCharacterWorld-Linux-DebugGame"
)

for binary in "${POSSIBLE_BINARIES[@]}"; do
  if [[ -x "$binary" ]]; then
    exec "$binary" "$@"
  fi
done

echo "Could not find a packaged Linux Unreal binary." >&2
echo "Package the project first, then rerun this script." >&2
exit 1
