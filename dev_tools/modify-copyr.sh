#!/bin/bash

NEW_PAR_FILE="new_par.txt"
MYPATH=.

# search in egrep form like    '.gif|.jpeg|.cpp'     if you search for extension
# use $ after every pattern in order to select the end of the filename (= extension)
SEARCH='.py$'
SEARCH='.rb$'
SEARCH='.cpp$|.hpp$|.i$'


while getopts "p:d:s:" flag ; do
    case $flag in
	p)  NEW_PAR_FILE="$OPTARG" ;;
	d)  MYPATH="$OPTARG" ;;
	s)  SEARCH="$OPTARG" ;;
	\?) echo "bash $0 -d targetdir -p new_paragraph_file -s search_file_extensions"; exit 1 ;;
    esac
done

find $MYPATH -type f | egrep -i $SEARCH | while read FILE; do

	mybegin=`    grep -n '^\/\*'  $FILE | head -1 | cut -f1 -d: `
#	mybegin=`    grep -n '^"""'  $FILE | head -1 | cut -f1 -d: `
#	mybegin=`    grep -n '^=begin'  $FILE | head -1 | cut -f1 -d: `
	mycopyright=`grep -n Copyright $FILE | head -1 | cut -f1 -d: `
	myend=`      grep -n '^\*\/$'  $FILE | head -1 | cut -f1 -d: `
#	myend=`      grep -n '^"""$'  $FILE | head -2 | tail -1 |cut -f1 -d: `
#	myend=`      grep -n '^=end'  $FILE | head -2 | tail -1 |cut -f1 -d: `


	if [ ! x`expr $mybegin + 1` =  x$mycopyright -o $mycopyright -gt $myend ]; then

		echo "$FILE       does not contain paragraph"

	else
		total=`wc -l $FILE  | awk '{ print $1}'`
		head -n `expr $mybegin - 1` $FILE > /tmp/.before.$$

		tail -n `expr $total - $myend` $FILE > /tmp/.after.$$

		cat /tmp/.before.$$ $NEW_PAR_FILE /tmp/.after.$$ >  $FILE
	fi
done

rm  /tmp/.before.$$ /tmp/.after.$$
