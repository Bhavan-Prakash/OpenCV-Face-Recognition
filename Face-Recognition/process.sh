#!/bin/bash

# Change to the directory where your photos and FaceRecognition executable are located
cd /home/jetson/MyDir/SeedoCheckinfile

# Loop through all JPG files in the directory
for file in *.png; do
  echo "Processing $file"
  ./FaceRecognition "$file"
done

