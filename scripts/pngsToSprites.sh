#!/usr/bin/env bash

# Exit immediately if a command fails
set -e

SPRITE_DIR="./realRaycast/graphics/$1"
SCRIPT="./scripts/pngToSprite.sh"

i=0
for png_file in "$SPRITE_DIR"/*.png; do
    # Skip the loop if no .png files are found
    [ -e "$png_file" ] || continue

    # If there are none or one set of arguments, give the same to all
    if [ ${#@} -le 2 ]; then
        "$SCRIPT" "$png_file" ${@:2} -r

    # If there are two sets of arguments, alternate which is given
    elif [ ${#@} -eq 3 ]; then
        # First set for even numbered iterations
        if [ $(( i % 2 )) -eq 0 ]; then
            "$SCRIPT" "$png_file" $2 -r
        else 
            "$SCRIPT" "$png_file" $3 -r
        fi
    fi

    # Don't add comma for last one
    if [ $i -lt 7 ]; then
        echo ,
    fi
    ((i++))
done