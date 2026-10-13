#!/bin/bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "This capture helper supports macOS only. Use a window screenshot tool on Windows/Linux." >&2
    exit 1
fi

if [[ $# -lt 1 || $# -gt 2 || ! "$1" =~ ^m[0-9]{3}-[a-z0-9]+(-[a-z0-9]+)*$ ]]; then
    echo "Usage: bash tools/capture-milestone.sh m001-native-window [app-pid]" >&2
    exit 1
fi
if [[ $# -eq 2 && ! "$2" =~ ^[0-9]+$ ]]; then
    echo "The optional app PID must be a number." >&2
    exit 1
fi

task_root="$(cd "$(dirname "$0")/.." && pwd)"
output="$task_root/docs/assets/milestones/$1.png"
if [[ -e "$output" ]]; then
    echo "Screenshot already exists: $output. Choose a new name to preserve milestone history." >&2
    exit 1
fi

mkdir -p "$task_root/build/swift-module-cache" "$(dirname "$output")"
window_id="$(/usr/bin/swift -module-cache-path "$task_root/build/swift-module-cache" \
    "$task_root/tools/find-app-window.swift" "${2:-}")"

if ! /usr/sbin/screencapture -x -o -l "$window_id" "$output"; then
    echo "Window capture failed. Check Screen Recording access for the terminal or host app in macOS System Settings." >&2
    exit 1
fi
if [[ ! -s "$output" ]]; then
    echo "Window capture produced no image." >&2
    exit 1
fi
printf '%s\n' "$output"
