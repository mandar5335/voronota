#!/bin/bash

cd $(dirname "$0")

ADDITION_TO_PATH=""
ADDITION_TO_LD_LIBRARY_PATH=""

while getopts "b:l:" OPTION
do
	case $OPTION in
	b)
		ADDITION_TO_PATH=$OPTARG
		;;
	l)
		ADDITION_TO_LD_LIBRARY_PATH=$OPTARG
		;;
	esac
done

if [ -n "$ADDITION_TO_PATH" ]
then
	export PATH=${ADDITION_TO_PATH}:${PATH}
fi

if [ -n "$ADDITION_TO_LD_LIBRARY_PATH" ]
then
	export LD_LIBRARY_PATH=${ADDITION_TO_LD_LIBRARY_PATH}:${LD_LIBRARY_PATH}
fi

time -p ./tests/run_all_tests.bash
