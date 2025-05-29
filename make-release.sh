#!/bin/bash
# Make temporary folder to add files into.
mkdir v-hud

# Copy the asi file.
cp output/asi/VHud.asi v-hud
cp resources/VHud\ Readme.txt v-hud
cp vendor/bass/bass.dll v-hud

# Now the assets.
cp -r resources/Extra v-hud/
cp -r resources/VHud v-hud/

# now zip the folder.
zip -r v-hud-v0.960-ryke.zip v-hud