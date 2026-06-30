#!/usr/bin/env bash
set -euo pipefail

port="${1:-}"

if [[ -z "$port" ]]; then
  port="$(ls /dev/tty.usbmodem* /dev/cu.usbmodem* 2>/dev/null | head -n 1 || true)"
fi

if [[ -z "$port" ]]; then
  echo "No USB serial device found. Connect the debug firmware over USB first." >&2
  exit 1
fi

out="twist-log-$(date +%Y%m%d-%H%M%S).txt"
echo "Reading $port at 115200 baud. Writing $out"
echo "Press Ctrl-C to stop."

stty -f "$port" 115200 raw -echo
cat "$port" | tee "$out"
