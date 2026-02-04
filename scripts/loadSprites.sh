#!/usr/bin/env bash

set -e

echo "loading sprites at $1"
echo "doing macros? $2"

i=0
echo "num args: $#"
for item in ${@:3}; do
    echo "iter $i $item"

    # Get args as all items past the name
    args=${item#*$SEPARATOR}

    echo "args: $args"

    # Get name as what comes before args
    name=${item:0:(( ${#item} - ${#args} - ${#SEPARATOR} ))}
    echo "name: $name"

    path=$1/$name
    echo "path $path"

    # Add macro for sprite indexing
    if [ $2 -eq 1 ] && [ ! -d "$path.anim" ]; then
        sprite_macros="${sprite_macros}#define sprite_$name $loaded_sprites\n"
        echo "added macro"
    fi

    # If is dir without .anim, load its images as a rotating sprite
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
            ./scripts/loadRotSprite.sh "$path" "${arg_set1//$SEPARATOR2/ }" >> $SPRITE_OUTPUT
        else
            # Call with both sets
            ./scripts/loadRotSprite.sh "$path" "${arg_set1//$SEPARATOR2/ }" "${arg_set2//$SEPARATOR2/ }" >> $SPRITE_OUTPUT
        fi
        loaded_sprites=$(( ${loaded_sprites} + ${ROT_SPRITE_FRAMES} ))
    
    # If dir is tagged as animation, load as so
    elif [ -d "$path.anim" ]; then
        echo "animation"

        # Remember vars because of recursive source vars
        prev_i=$i
        source ./scripts/loadAnim.sh "$path.anim" ${args//$SEPARATOR2/ }
        i=$prev_i
        ((loaded_anims++))

    # Otherwise, load single image
    elif [ -e "$path.png" ]; then
        echo "image"
        source ./scripts/pngToSprite.sh "$path.png" "${args//$SEPARATOR2/ }" >> $SPRITE_OUTPUT
        ((loaded_sprites++))

    else
        echo "error: $path does not exist"
        exit 1
    fi

    if [ $(( $i + 1 )) -lt $(( $# - 2 )) ]; then
        echo , >> $SPRITE_OUTPUT
        echo "added comma"
    fi
    ((i++))
done