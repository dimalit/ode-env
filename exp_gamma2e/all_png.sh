#!/bin/bash
for csv in *.csv
do
	base=`basename $csv .csv`
	png="${base}.png"
	gnuplot -e "set terminal png size 400,300; set xrange [0:5.05]; set yrange [0:1]; set output '$png'; plot '$csv' with lines title '$base'"
done
