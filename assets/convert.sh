for img in originals/*.png; do
    magick "$img" -crop 707x701+158+166 -resize 128x128 "${img%.png}_resized.png"
done

