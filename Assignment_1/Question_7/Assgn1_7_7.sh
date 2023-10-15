[ -d "$2" ] || mkdir $2&&for file in $1/*
do
    IFS=$'\n'&&while read line || [ "$line" ]
    do
        echo "$line" >> $2/${line:0:1}.txt
    done < "$file"
done&&for x in {A..Z}
do touch "$2/$x.txt"&&sort "$2/$x.txt" -o "$2/$x.txt"
done