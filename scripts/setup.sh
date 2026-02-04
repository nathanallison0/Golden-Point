#!/usr/bin/env bash
./scripts/loadAllSprites.sh \
laserShot:0.025f,-y,-50 \
smoke:0.05f,-y,-25 \
plant:0.7f \
guyForGameLaser:0.75f,-s \
boxShaded:0.4f:0.5f,-y,-5 \
laserHit:2,0.1f,-y,-50

echo "sprites finished"

./scripts/loadImages.sh sky laserForGame
echo "images finished"