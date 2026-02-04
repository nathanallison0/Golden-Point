#!/usr/bin/env bash

set -e

ANIM_DIR=$1
SUB_FRAMES=$2

LOAD_ARGS="${@:3}"
LOAD_ARGS="${LOAD_ARGS// /$SEPARATOR2}"

# Compile all file names into space separated list
script_args="$ANIM_DIR 0"
num_frames=0
for png_file in "$ANIM_DIR"/*.png; do
    # Remove file extension
    FNAME=${png_file%.png}

    # Add load args to end of each, stripping path from file
    script_args="$script_args ${FNAME##*/}$SEPARATOR$LOAD_ARGS"

    ((num_frames++))
done

# Add anim desc
echo -e -n ",\n\t{$loaded_sprites, $num_frames, $SUB_FRAMES}" >> $ANIM_OUTPUT

# Load anim images
echo "script args: '$script_args'"
# Don't do macros
source ./scripts/loadSprites.sh $script_args

# Add macro
# Remove path and .anim from path to get name
anim_file="${ANIM_DIR##*/}"
anim_macros="$anim_macros\n#define anim_${anim_file%.anim} $loaded_anims"