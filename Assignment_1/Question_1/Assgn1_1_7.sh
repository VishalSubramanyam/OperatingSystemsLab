function hcf { test "$2" = "0"&&echo $1||hcf $2 $(($1%$2))
}
l=1&&rev $1|while read line
do l=$(((l*line)/$(hcf l line)))
done&&echo $l