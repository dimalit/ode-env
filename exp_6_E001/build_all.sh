#!/bin/bash
for f in *_phi.png *_E.png
do
convert $f -transparent white $f
done

for t in te tm
do
for g in 0.1 1.1
do

composite ${t}_g${g}_r0.3_phi.png ${t}_g${g}_r1_phi.png ${t}_g${g}_phi.png
composite ${t}_g${g}_r3_phi.png ${t}_g${g}_phi.png ${t}_g${g}_phi.png

composite ${t}_g${g}_r0.3_E.png ${t}_g${g}_r1_E.png ${t}_g${g}_E.png
composite ${t}_g${g}_r3_E.png ${t}_g${g}_E.png ${t}_g${g}_E.png

montage	${t}_g${g}_E.png ${t}_g${g}_phi.png \
          -geometry +0+0 -tile 1x2 ${t}_g${g}_2.png

rm ${t}_g${g}_E.png ${t}_g${g}_phi.png

# rename begin/end to 1/2 if needed
for r in 0.3 1 3
do
	if [ ! -f ${t}_g${g}_r${r}_field1.png ]; then
		mv ${t}_g${g}_r${r}_field_begin.png ${t}_g${g}_r${r}_field1.png
		mv ${t}_g${g}_r${r}_field_end.png ${t}_g${g}_r${r}_field2.png
	fi
	if [ ! -f ${t}_g${g}_r${r}_field2.png ]; then
		mv ${t}_g${g}_r${r}_field_end.png ${t}_g${g}_r${r}_field2.png
	fi
done

montage ${t}_g${g}_r0.3_field1.png ${t}_g${g}_r0.3_field2.png \
	${t}_g${g}_r1_field1.png ${t}_g${g}_r1_field2.png \
	${t}_g${g}_r3_field1.png ${t}_g${g}_r3_field2.png \
          -geometry +0+0 -tile 2x3 ${t}_g${g}_6.png

mkdir out
cp ${t}_g${g}_2.png ${t}_g${g}_6.png out/

done
done
