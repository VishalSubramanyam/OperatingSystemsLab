while read u
do [[ $u =~ ^[a-zA-Z0-9]{5,20}$ ]]&&[[ $u =~ ^[a-zA-Z] ]]&&[[ $u =~ [0-9] ]]&&! grep -iqoFf "fruits.txt"<<<"$u"&&echo YES >> "validation_results.txt"||echo NO >> "validation_results.txt"
done <$1