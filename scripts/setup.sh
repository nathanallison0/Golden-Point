#!/usr/bin/env bash
./scripts/loadSprites.sh laserShot:0.025f,-y,-50 smoke:0.05f,-y,-25 plant:0.7f guyForGame:0.7f boxShaded:0.4f:0.5f,-y,-5
echo "sprites finished"
./scripts/loadImages.sh sky laserForGame
echo "images finished"