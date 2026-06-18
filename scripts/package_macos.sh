#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT_FILE="$PROJECT_ROOT/IpcCharacterWorld.uproject"

ENGINE_ROOT="${UE_ENGINE_ROOT:-}"
CONFIGURATION="Development"
ARCHIVE_DIRECTORY="$PROJECT_ROOT/Game/Mac"

usage() {
  cat <<'EOF'
Usage: scripts/package_macos.sh [--engine-root PATH] [--configuration Development|Shipping] [--archive-directory PATH]

Defaults:
  --configuration       Development
  --archive-directory   ./Game/Mac

Engine root resolution:
  1. --engine-root PATH
  2. UE_ENGINE_ROOT environment variable
  3. common macOS paths such as /Users/Shared/Epic Games/UE_5.7 or ~/UnrealEngine
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --engine-root)
      ENGINE_ROOT="${2:-}"
      shift 2
      ;;
    --configuration)
      CONFIGURATION="${2:-}"
      shift 2
      ;;
    --archive-directory)
      ARCHIVE_DIRECTORY="${2:-}"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if [[ ! -f "$PROJECT_FILE" ]]; then
  echo "Project file not found: $PROJECT_FILE" >&2
  exit 1
fi

if [[ -z "$ENGINE_ROOT" ]]; then
  for candidate in \
    "/Users/Shared/Epic Games/UE_5.7" \
    "$HOME/Epic Games/UE_5.7" \
    "$HOME/EpicGames/UE_5.7" \
    "$HOME/UnrealEngine" \
    "/Applications/UnrealEngine"; do
    if [[ -x "$candidate/Engine/Build/BatchFiles/RunUAT.sh" ]]; then
      ENGINE_ROOT="$candidate"
      break
    fi
  done
fi

RUN_UAT="$ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
if [[ -z "$ENGINE_ROOT" || ! -x "$RUN_UAT" ]]; then
  echo "Unreal RunUAT.sh not found. Pass --engine-root PATH or set UE_ENGINE_ROOT." >&2
  exit 1
fi

mkdir -p "$ARCHIVE_DIRECTORY"

echo "Packaging macOS build"
echo "  Engine:  $ENGINE_ROOT"
echo "  Project: $PROJECT_FILE"
echo "  Output:  $ARCHIVE_DIRECTORY"

"$RUN_UAT" BuildCookRun \
  -project="$PROJECT_FILE" \
  -noP4 \
  -platform=Mac \
  -clientconfig="$CONFIGURATION" \
  -build \
  -cook \
  -stage \
  -pak \
  -archive \
  -archivedirectory="$ARCHIVE_DIRECTORY"
