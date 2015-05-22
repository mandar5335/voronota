#!/bin/bash

set +e

readonly TMPDIR=$(mktemp -d)
trap "rm -r $TMPDIR" EXIT

ROOTDIR=""
REFERENCE_THRESHOLD="0.4"

while getopts "d:t:" OPTION
do
	case $OPTION in
	d)
		ROOTDIR=$OPTARG
		;;
	t)
		REFERENCE_THRESHOLD=$OPTARG
		;;
	esac
done

find $ROOTDIR -type f -name residue_cad_scores -not -empty \
| sed "s/residue_cad_scores$//" \
| awk '{print $1 "residue_cad_scores " $1 "residue_quality_scores"}' \
| $BINDIR/voronota score-scores \
  --reference-threshold $REFERENCE_THRESHOLD \
> $TMPDIR/local_scores_evaluation

{
	echo target $(cat $TMPDIR/local_scores_evaluation | awk '{print $1}')
	echo $(basename $ROOTDIR) $(cat $TMPDIR/local_scores_evaluation | awk '{print $2}')
} > $ROOTDIR/local_scores_evaluation
