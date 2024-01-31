#!/bin/bash
# This is a helper script to convert .pngs produced by ARGoS into a .mp4

## Format File Names for .mp4 conversion
echo "Formatting File Names For Conversion..."
for filename in *.png; 
do  
    # Check if file is a regular file, skip if not
    [ -f "$filename" ] || continue;    

    # Check if the file starts with the header to be removed, if so remove it. 
    if [[ "$filename" == frame_* ]]; then
        mv "$filename" "${filename//frame_/}";
    fi
done

## Optionally remove first 45 seconds of video.
if_remove_45s=0
while true; 
do
  # Ask the user if they want to delete all the files
  read -p "Do you the first 45 seconds of the video removed? (y/n)" answer

  # Check the answer and delete the files if yes
  case $answer in
    [Yy]* ) echo "Removing the first 45 seconds of the video..."; if_remove_45s=1; break;;
    [Nn]* ) echo "No portion of the video will be deleted."; break;;
    * ) echo "Invalid input. Please try again.";;
  esac;
done

# Used to get rid of the first 450 pngs, renames the files to appropriate names for the next session.
if [[ $if_remove_45s -eq 1 ]]; then
    for filename in *.png; 
    do      
        # Extract the number from the filename
        number=$(echo "$filename" | sed 's/[^0-9]*//g')

        if [[ -z "$number" ]]; then
            # Do something if it is
            echo "The variable is empty"
            exit 1
        fi


        # Rename the file with the new number or delete if it's less than the desired number
        if [ $number -lt 450 ]; then
            rm "$filename"
            continue
        fi  

        # Subtract 450 from the number
        new_number=$((10#$number - 10#0000000450))

        # Rename file with the new number
        mv "$filename" "$(printf "%010d" "$new_number").png" 
    done
    echo "Removed the first 45 seconds worth of .png files."
fi
    
## Convert .png files into .mp4
echo "Converting to .mp4..."
ffmpeg -r 10 -f image2 -s 1299x922 -i %10d.png -vcodec libx264 -vf "pad=ceil(iw/2)*2:ceil(ih/2)*2" -crf 25  -pix_fmt yuv420p argos_recording.mp4
echo "Conversion completed."

## Optionally delete .png files after conversion.
while true; 
do
  # Ask the user if they want to delete all the files
  read -p "Do you want to delete all the .png files? (y/n)" answer

  # Check the answer and delete the files if yes
  case $answer in
    [Yy]* ) echo "Deleting all files..."; rm *.png; echo "All files deleted."; break;;
    [Nn]* ) echo "No files deleted."; break;;
    * ) echo "Invalid input. Please try again.";;
  esac;
done