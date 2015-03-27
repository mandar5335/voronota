#!/bin/bash

LISTFILE=""
ENDING=""
OUTFILE=""

while getopts "l:e:o:" OPTION
do
	case $OPTION in
	l)
		LISTFILE=$OPTARG
		;;
	e)
		ENDING=$OPTARG
		;;
	o)
		OUTFILE=$OPTARG
		;;
	?)
		echo "Unrecognized option." 1>&2
		exit 1
		;;
	esac
done

mkdir -p $(dirname $OUTFILE)

(cat $LISTFILE | sed "s/\(.*\)/\1\/$ENDING/" | xargs -L 100 -P 1 cat) > $OUTFILE
