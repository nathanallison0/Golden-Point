#!/usr/bin/env bash
SEPARATOR=:
SEPARATOR2=,
export ROT_SPRITE_FRAMES=8

loaded_sprites=0
sprite_macros=""
SPRITE_OUTPUT="./realRaycast/spritesRaw.h"
#SPRITE_OUTPUT="./spritesTest.txt"

loaded_anims=0
anim_macros=""
ANIM_OUTPUT="./realRaycast/animDescsRaw.h"
#ANIM_OUTPUT="./animTest.txt"

# Clear and prepare sprite array
echo "sprite sprites[] = {" > $SPRITE_OUTPUT
echo -e "\t#if !__VSCODE__" >> $SPRITE_OUTPUT

# Clear and prepare animations
# Add line comment to catch first comma
echo -e -n "anim_desc anim_descs[] = {\n//" > $ANIM_OUTPUT

# Do load macros
source ./scripts/loadSprites.sh ./realRaycast/graphics 1 $@

# Close sprite array and paste macros
echo -e "\n\t#endif" >> $SPRITE_OUTPUT
echo "};" >> $SPRITE_OUTPUT
echo -e $sprite_macros >> $SPRITE_OUTPUT

echo "// loaded $loaded_sprites sprites" >> $SPRITE_OUTPUT

# Close anim array and paste macros
echo -e "\n};" >> $ANIM_OUTPUT
echo -e "$anim_macros" >> $ANIM_OUTPUT

echo "// loaded $loaded_anims anims" >> $ANIM_OUTPUT