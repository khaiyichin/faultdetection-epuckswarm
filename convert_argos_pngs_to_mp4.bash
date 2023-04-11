#!/bin/bash

for filename in *.png; do      [ -f "$filename" ] || continue;     mv "$filename" "${filename//frame_/}"; done

ffmpeg -r 10 -f image2 -s 1299x922 -i %10d.png -vcodec libx264 -vf "pad=ceil(iw/2)*2:ceil(ih/2)*2" -crf 25  -pix_fmt yuv420p argos_recording.mp4
