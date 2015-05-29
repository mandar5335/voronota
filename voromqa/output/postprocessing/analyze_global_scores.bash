#!/bin/bash

SCRIPTDIR=$(dirname $0)
INFILE=$1
OUTDIR=$2

function analyze
{
	local TESTSCORE=$1
	local REFSCORE=$2
	local FLAGS=$3
	local SUBDIR=$OUTDIR/${TESTSCORE}__vs__${REFSCORE}
	mkdir -p $SUBDIR
	R --vanilla --args \
	  V-input $INFILE $FLAGS \
	  V-testscore-name $TESTSCORE \
	  V-refscore-name $REFSCORE \
	  V-pdf-output $SUBDIR/plots.pdf \
	  F-plot-per-target \
	< $SCRIPTDIR/analyze_global_scores.R \
	| grep 'Results statistics output' -A 9999 \
	> $SUBDIR/log
}

analyze "qscore_atom" "cadscore_residue"
analyze "qscore_atom" "tmscore"
analyze "qscore_atom" "goap" "F-invert-refscore F-normalize-refscore"
analyze "qscore_atom" "dfire" "F-invert-refscore F-normalize-refscore"
analyze "qscore_atom" "goap_ag" "F-invert-refscore F-normalize-refscore"
analyze "qscore_atom" "rwplus" "F-invert-refscore F-normalize-refscore"

analyze "qenergy" "cadscore_residue" "F-invert-testscore F-normalize-testscore-by-area"
analyze "qenergy" "tmscore" "F-invert-testscore F-normalize-testscore-by-area"

analyze "goap" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "goap" "tmscore" "F-invert-testscore F-normalize-testscore"

analyze "dfire" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "dfire" "tmscore" "F-invert-testscore F-normalize-testscore"

analyze "goap_ag" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "goap_ag" "tmscore" "F-invert-testscore F-normalize-testscore"

analyze "rwplus" "cadscore_residue" "F-invert-testscore F-normalize-testscore"
analyze "rwplus" "tmscore" "F-invert-testscore F-normalize-testscore"
