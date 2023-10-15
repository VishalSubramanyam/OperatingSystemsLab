#!/bin/bash
declare -A major_count student_major
while read -r name major
do
    major_count[$major]=$((major_count[$major]+1))
    student_major[$name]+=" $major"
done < "$1"
for major in "${!major_count[@]}"; do
    echo "$major ${major_count[$major]}"
done | sort -k2nr -k1

single_major_count=0&&for student in "${!student_major[@]}"
do [ $(echo ${student_major[$student]}|wc -w) -gt 1 ] &&echo $student
done
for student in "${!student_major[@]}"; do [ $(wc -w <<< ${student_major[$student]}) -eq 1 ] &&single_major_count=$((single_major_count+1))
done&&echo "$single_major_count"