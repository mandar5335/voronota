#!/bin/bash

SUBDIR=$OUTPUTDIR/convenience_contacts_script
mkdir -p $SUBDIR

$VORONOTADIR/voronota-contacts \
  -i $INPUTDIR/single/structure.pdb \
  --atoms-query "--match r<1:50>" \
  --contacts-query "--inter-residue" \
  --cache-dir $SUBDIR/cache \
> $SUBDIR/contacts0

$VORONOTADIR/voronota-contacts \
  --input $INPUTDIR/single/structure.pdb \
  --atoms-query "--match r<1:50>" \
  --contacts-query "--inter-residue --match-first R<LYS>" \
  --output-drawing $SUBDIR/contacts1_drawing.py \
  --drawing-parameters "--default-color 0x00FFFF --drawing-name contacts1" \
  --cache-dir $SUBDIR/cache \
> $SUBDIR/contacts1

$VORONOTADIR/voronota-contacts \
  --input $INPUTDIR/single/structure.pdb \
  --atoms-query "--match r<1:50>" \
  --contacts-query "--inter-residue --match-first R<LYS>" \
  --wireframe-drawing \
  --output-drawing $SUBDIR/contacts1_wireframe_drawing.py \
  --drawing-parameters "--default-color 0xFF00FF --drawing-name contacts1_wireframe" \
  --cache-dir $SUBDIR/cache \
> /dev/null

$VORONOTADIR/voronota-contacts \
  --input $INPUTDIR/single/structure.pdb \
  --atoms-query "--match r<1:50>" \
  --contacts-query "--inter-residue --match-first R<LEU>" \
  --cache-dir $SUBDIR/cache \
> $SUBDIR/contacts2

rm -r $SUBDIR/cache
