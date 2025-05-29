#!/bin/bash
# Make temporary folder to add files into.
mkdir v-hud

# Copy the asi file.
cp output/asi/VHud.asi v-hud

# Now the assets.
cp -r resources v-hud/

# now zip the folder.
zip -r v-hud-v0.960-ryke.zip v-hud