cd /home/jetson/MyDir/SeedoCheckinfile


for file in *.jpg; do
  echo "Processing $file"
  ./FaceRecognition "$file"
done

