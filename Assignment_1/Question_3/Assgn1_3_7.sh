for file in $1/*.jsonl; do
    a=$2/`basename $file .jsonl`.csv; # name of the output csv file based on the JSON file
    b=${@:3}; # arguments starting from the third one onwards
    echo $b | tr ' ' ',' > $a; # output column headers into CSV file
    jq -r [`echo $b|sed 's/^/./;s/ /,./g'`]'|@csv'<$file>>$a
done