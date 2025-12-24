#!/usr/bin/env bash

set -e

SPRITE_DIR="./realRaycast/sprites"
OUTPUT="./realRaycast/spritesRaw2.h"
SEPARATOR=:
ROT_SPRITE_FRAMES=8

echo "array_sprite array_sprites[] = {" > $OUTPUT
echo -e "\t#if !__VSCODE__" >> $OUTPUT

i=0
loaded=0
macros=""
echo "num args: $#"
for item in $@; do
    echo "iter $i $item"

    args=${item#*$SEPARATOR}
    # Replace serparators with spaces for arg use
    args=${args//$SEPARATOR/ }

    echo "args: $args"

    name=${item:0:(( ${#item} - ${#args} - ${#SEPARATOR}))}
    echo "name: $name"

    macros="${macros}#define sprite_${name} ${loaded}\n"
    path=$SPRITE_DIR/$name
    echo "path $path"

    # If is dir, load its images
    if [ -d "$path" ]; then
        echo "dir"
        ./scripts/pngsToSprites.sh $name $args >> $OUTPUT
        loaded=$(( ${loaded} + ${ROT_SPRITE_FRAMES} ))
    
    # Otherwise, load single image
    elif [ -e "$path.png" ]; then
        echo "image"
        ./scripts/pngToSprite.sh $path.png $args >> $OUTPUT
        ((loaded++))

    else
        echo "error: $path does not exist"
        exit 1
    fi

    if [ $(( $i + 1 )) -lt "$#" ]; then
        echo , >> $OUTPUT
        echo "added comma"
    fi
    ((i++))
done

echo -e "\n\t#endif" >> $OUTPUT
echo "};" >> $OUTPUT
echo -e $macros >> $OUTPUT