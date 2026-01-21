#!/usr/bin/env bash

set -e

SPRITE_DIR="./realRaycast/graphics"
OUTPUT="./realRaycast/spritesRaw.h"
#OUTPUT=test.txt
SEPARATOR=:
SEPARATOR2=,
ROT_SPRITE_FRAMES=8

echo "sprite sprites[] = {" > $OUTPUT
echo -e "\t#if !__VSCODE__" >> $OUTPUT

i=0
loaded=0
macros=""
echo "num args: $#"
for item in $@; do
    echo "iter $i $item"

    # Get args as all items past the name
    args=${item#*$SEPARATOR}

    echo "args: $args"

    # Get name as what comes before args
    name=${item:0:(( ${#item} - ${#args} - ${#SEPARATOR} ))}
    echo "name: $name"

    # Add macro for sprite indexing
    macros="${macros}#define sprite_${name} ${loaded}\n"

    path=$SPRITE_DIR/$name
    echo "path $path"

    # If is dir, load its images as a rotating sprite
    if [ -d "$path" ]; then
        # Separate two sets of arguments
        arg_set1=$args
        arg_set2="undef"

        # If set1 contains separator, get set2
        if [[ $arg_set1 == *$SEPARATOR* ]]; then
            arg_set2=${arg_set1#*$SEPARATOR}
            # Cut off set2 from set1
            arg_set1=${arg_set1:0:(( ${#arg_set1} - ${#arg_set2} - ${#SEPARATOR} ))}
        fi

        echo "set1: '$arg_set1'"
        echo "set2: '$arg_set2'"
        
        if [[ $arg_set2 == "undef" ]]; then
            # Call with first set only
            ./scripts/pngsToSprites.sh $name "${arg_set1//$SEPARATOR2/ }" >> $OUTPUT
        else
            # Call with both sets
            ./scripts/pngsToSprites.sh $name "${arg_set1//$SEPARATOR2/ }" "${arg_set2//$SEPARATOR2/ }" >> $OUTPUT
        fi
        loaded=$(( ${loaded} + ${ROT_SPRITE_FRAMES} ))
    
    # Otherwise, load single image
    elif [ -e "$path.png" ]; then
        echo "image"
        ./scripts/pngToSprite.sh $path.png "${args//$SEPARATOR2/ }" >> $OUTPUT
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