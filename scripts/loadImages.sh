#!/usr/bin/env bash

set -e

IMAGE_DIR="./realRaycast/graphics"
OUTPUT="./realRaycast/imagesRaw.h"

# Declare array
echo -e "image images[] = {\n\t#if !__VSCODE__" > $OUTPUT

macros=""
i=0
for image in $@; do
    # Add macro
    macros="${macros}#define image_$image $i\n"

    path=$IMAGE_DIR/$image.png
    echo "path: $path"
    if [ -e $path ]; then
        scripts/pngToImage.sh $path >> $OUTPUT
    else
        echo "error: $path does not exist"
        exit 1
    fi

    if [ $(( $i + 1 )) -lt "$#" ]; then
        echo , >> $OUTPUT
    fi

    ((i++))
done

# Close array
echo -e "\n\t#endif\n};" >> $OUTPUT

# Add macros
echo -e "$macros" >> $OUTPUT