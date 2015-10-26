#!/bin/bash

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

INPUT_FILE_LIST=""
OUTPUT_FILE=""
HEADER=false
SORT=false
TABULATE=false
COMPRESS=false
ANNOTATE=false

while getopts "i:o:hstza" OPTION
do
	case $OPTION in
	i)
		INPUT_FILE_LIST=$OPTARG
		;;
	o)
		OUTPUT_FILE=$OPTARG
		;;
	h)
		HEADER=true
		;;
	s)
		SORT=true
		;;
	t)
		TABULATE=true
		;;
	z)
		COMPRESS=true
		;;
	a)
		ANNOTATE=true
		;;
	esac
done

if $ANNOTATE
then
	cat $INPUT_FILE_LIST | while read INPUT_FILE
	do
		echo "${INPUT_FILE}:"
		cat $INPUT_FILE
	done > $TMPDIR/output
else
	(cat $INPUT_FILE_LIST | xargs -L 100 -P 1 cat) > $TMPDIR/output
fi

if $HEADER
then
	head -1 $TMPDIR/output > $TMPDIR/header
	cat $TMPDIR/output | egrep -v "$(cat $TMPDIR/header)" > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

if $SORT
then
	cat $TMPDIR/output | sort | uniq > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

if $HEADER
then
	cat $TMPDIR/header $TMPDIR/output > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

if $TABULATE
then
	cat $TMPDIR/output | column -t > $TMPDIR/output_mod
	mv $TMPDIR/output_mod $TMPDIR/output
fi

mv $TMPDIR/output $OUTPUT_FILE

if $COMPRESS
then
	bzip2 $OUTPUT_FILE
fi
