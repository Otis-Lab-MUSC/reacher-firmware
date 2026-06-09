#!/bin/bash
# Compile all REACHER firmware paradigms for every supported board.
# Requires: arduino-cli with arduino:avr board package installed.
#
# Usage:  bash compile.sh
# Output: hex/<board>/<paradigm>.hex   for each (paradigm, board) pair.
#         Subdirectory layout matches the reacher backend uploader's
#         resolver (uploader/uploader.py::get_hex_path).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
HEX_DIR="$SCRIPT_DIR/hex"
LIB_DIR="$SCRIPT_DIR/libraries"

mkdir -p "$HEX_DIR"

for board in mega; do
    case "$board" in
        mega) FQBN="arduino:avr:mega:cpu=atmega2560" ;;
    esac

    BOARD_DIR="$HEX_DIR/$board"
    mkdir -p "$BOARD_DIR"

    for sketch in fr pr vi omission pavlovian; do
        echo "==> Compiling $sketch for $board ($FQBN)..."
        arduino-cli compile \
            --fqbn "$FQBN" \
            --libraries "$LIB_DIR" \
            --output-dir "$BOARD_DIR" \
            "$SCRIPT_DIR/$sketch/$sketch.ino"

        # arduino-cli names the output <sketch>.ino.hex — rename to <sketch>.hex
        if [ -f "$BOARD_DIR/$sketch.ino.hex" ]; then
            mv "$BOARD_DIR/$sketch.ino.hex" "$BOARD_DIR/$sketch.hex"
        fi

        # Clean up extra build artifacts
        rm -f "$BOARD_DIR/$sketch.ino.elf" "$BOARD_DIR/$sketch.ino.with_bootloader.hex"

        echo "    -> $BOARD_DIR/$sketch.hex"
    done
done

echo ""
echo "All paradigms compiled successfully for MEGA."
ls -lh "$HEX_DIR"/*/*.hex
