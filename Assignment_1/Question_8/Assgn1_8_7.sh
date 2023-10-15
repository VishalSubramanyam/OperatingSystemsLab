if [ ! -e main.csv ]; then
    touch "main.csv"
    echo "date,category,amount,name" > "main.csv"
fi

insert() {
    echo "$1,$2,$3,$4" >> "main.csv"
    echo "Inserted $1 in main.csv"
}

category_spent() {
    spent=$(awk -F, -v cat="$1" 'BEGIN {sum=0} NR > 1 && $2 == cat {sum+=$3} END {print sum}' "main.csv")
    echo "Total amount spent on $1 is $spent"
}

name_spent() {
    spent=$(awk -F, -v name="$1" 'BEGIN {sum=0} NR > 1 &&  $4 == name {sum+=$3} END {print sum}' "main.csv")
    echo "Total amount spent by $1 is $spent"
}

sort_csv() {
    column=$1
    filename=$2
    headerline=$(head -n 1 $filename)
    tail -n +2 $filename | sort -t, -k${column},${column} | (echo $headerline; cat) > sorted.csv
    mv sorted.csv $filename
}

print_help() {
    echo "NAME"
    echo "    Travel Expense Tracker"
    echo "SYNOPSIS"
    echo "    sh Assgn1_8_7.sh [-c category] [-n name] [-s column] [-h] [record]"
    echo "DESCRIPTION"
    echo "    A travel expenses tracker which lets you insert entries and perform various operations depending on the options"
    echo "    -c category"
    echo "          print the total amount of money spent for the given category"
    echo "    -n name"
    echo "          print the total amount of money spent by the person with that name"
    echo "    -s column"
    echo "          sort the csv file according to the column given"
    echo "    -h"
    echo "          displays this help message"
}

given_date=""
given_name=""
given_amount=""
given_cat=""
category=""
name=""
column=0
help_needed=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c)
            shift
            category=$1
            shift
            ;;
        -n)
            shift
            name=$1
            shift
            ;;
        -s)
            shift
            if [ $1 == "date" ]; then
                column=1
            fi
            if [ $1 == "category" ]; then
                column=2
            fi
            if [ $1 == "amount" ]; then
                column=3
            fi
            if [ $1 == "name" ]; then
                column=4
            fi
            shift
            ;;
        -h)
            shift
            help_needed=1
            ;;
        *)
            given_date="$1"
            given_cat="$2" 
            given_amount="$3"
            given_name="$4"
            shift 4
            ;;
    esac
done


[[ -z  $given_date ]] || insert $given_date $given_cat $given_amount $given_name

[[ -z $name ]] || name_spent $name

[[ -z $category ]] || category_spent "$category"

if [ ! $column -eq 0 ]; then 
    sort_csv $column "main.csv"
else
    sort_csv 1 main.csv
fi


if [[ $help_needed -eq 1 ]]; then
    print_help
fi
