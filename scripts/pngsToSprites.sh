#!/usr/bin/env bash

# Exit immediately if a command fails
set -e

SPRITE_DIR="./realRaycast/sprites/$1"
SCRIPT="./scripts/pngToSprite.sh"

i=0
for png_file in "$SPRITE_DIR"/*.png; do
    # Skip the loop if no .png files are found
    [ -e "$png_file" ] || continue

    "$SCRIPT" "$png_file" ${@:2}

    # Don't add comma for last one
    if [ $i -lt 7 ]; then
        echo ,
    fi
    ((i++))
done