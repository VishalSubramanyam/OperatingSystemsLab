#!/bin/bash
directory=$1
find $directory -name "*.py" -print0 | while IFS= read -r -d '' file; do
    echo "File: $file"
    grep -n "#" $file
    awk '/^"""/ {print NR;next} {print}' $file | grep -n "\"\"\""
done
