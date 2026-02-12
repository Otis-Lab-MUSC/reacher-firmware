#!/bin/bash
# Compile all REACHER firmware paradigms to .hex files.
# Requires: arduino-cli with arduino:avr board package installed.
#
# Usage:  bash compile.sh
# Output: hex/fr.hex  hex/pr.hex  hex/vi.hex  hex/omission.hex  hex/pavlovian.hex

set -euo pipefail

FQBN="arduino:avr:uno"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
HEX_DIR="$SCRIPT_DIR/hex"
LIB_DIR="$SCRIPT_DIR/libraries"

mkdir -p "$HEX_DIR"

for sketch in fr pr vi omission pavlovian; do
    echo "==> Compiling $sketch..."
    arduino-cli compile \
        --fqbn "$FQBN" \
        --libraries "$LIB_DIR" \
        --output-dir "$HEX_DIR" \
        "$SCRIPT_DIR/$sketch/$sketch.ino"

    # arduino-cli names the output <sketch>.ino.hex — rename to <sketch>.hex
    if [ -f "$HEX_DIR/$sketch.ino.hex" ]; then
        mv "$HEX_DIR/$sketch.ino.hex" "$HEX_DIR/$sketch.hex"
    fi

    # Clean up extra build artifacts
    rm -f "$HEX_DIR/$sketch.ino.elf" "$HEX_DIR/$sketch.ino.with_bootloader.hex"

    echo "    -> $HEX_DIR/$sketch.hex"
done

echo ""
echo "All paradigms compiled successfully."
ls -lh "$HEX_DIR"/*.hex
