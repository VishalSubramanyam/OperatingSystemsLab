while IFS= read -r line
do
echo "$line"|grep -qe "\b$2\b"&&echo "$line" | sed 's/[A-Za-z]/&\n/g' | awk '{if(NR%2) printf "%s",toupper($0)
else printf "%s",tolower($0)}'&&echo ||
echo "$line"
done<$1