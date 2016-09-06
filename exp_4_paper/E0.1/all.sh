#!/bin/bash
cd 1
gnuplot ../repeated.gnuplot
cd ..

cd 2
gnuplot ../repeated.gnuplot
cd ..

cd 3
gnuplot ../repeated.gnuplot
cd ..

gnuplot common.gnuplot

