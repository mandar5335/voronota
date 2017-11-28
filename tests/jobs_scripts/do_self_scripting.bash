#!/bin/bash

SUBDIR=$OUTPUTDIR/self_scripting
mkdir -p $SUBDIR

{
cat << EOF
load-atoms file $INPUTDIR/single/structure.cif format mmcif
load-atoms file $INPUTDIR/single/structure.pdb
load-atoms file $INPUTDIR/single/structure.pdb include-heteroatoms
print-atoms {tags het adjuncts tf=0:10}
restrict-atoms {tags-not het}
construct-contacts calculate-volumes
save-atoms file '$SUBDIR/plain_atoms'
save-contacts file '$SUBDIR/plain_contacts'
load-atoms file '$SUBDIR/plain_atoms' format plain
select-contacts
load-contacts file '$SUBDIR/plain_contacts'
select-contacts {atom-first {match R<PHE>} atom-second {match R<PHE>} min-area 5.0 min-seq-sep 1} name cs1
print-contacts {sel cs1} sort-r area
print-contacts {no-solvent min-seq-sep 2} sort-r area limit 3 expand
print-contacts {no-solvent min-seq-sep 2} sort-r area limit 3 expand inter-residue
select-atoms {match r<64>&A<C,N,O,CA,CB>} name as1
print-atoms {sel as1} sort tags
print-atoms {match r<64>&A<C,N,O,CA,CB>} sort atmn expand
rename-selection-of-atoms nosel1 nodel2
delete-selections-of-contacts nosel1
list-selections-of-atoms
list-selections-of-contacts
print-history last 5
print-history
EOF
} \
| $VORONOTA x-run-script \
> $SUBDIR/script_output \
2> $SUBDIR/script_errors
