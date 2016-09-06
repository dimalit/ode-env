#!/bin/bash
for f in *gnuplot
do
	base=`basename $f .gnuplot`
	full="${base}.png"
	xrange='[*:*]'
	yrange='[*:*]'
	if [[ $base == *_E ]]
	then
		xrange='[0:120]'
		yrange='[0:1.1]'
	fi
	if [[ $base == *_phi ]]
	then
		xrange='[0:120]'
		yrange='[-4:7]'
	fi
	if [[ $base == *_field* ]]
	then
		xrange='[-0.5:0.5]'
		yrange='[0:1.5]'
	fi
	gnuplot -e "set terminal png size 400,300; set xrange $xrange; set yrange $yrange; set output '$full'" $f
done
