reset
set term post color 
set out 'subgroups.eps'

set grid
set multiplot
set yrange[300:1000]
set key bottom right
set label 1 "Run time (secs)" at graph -0.1, graph 0.5 center rotate
#set ylabel "Run time (secs)"
set xlabel "Subgroup ID"
set xtics 5
set ytics (400, 500, 600, 900)
set size 1,0.5
set origin 0.0,0.0
set bmargin 3
set lmargin 5
set tmargin 0
plot "160k.masterEnd.dat" using 1:2 title "With pairs redistribution" w lp lw 3 pt 1, \
"masterEnd.noSP.dat" using 1:2 title "Without pairs redistribution" w lp lw 3 pt 4
set yrange [1000:2500]
set ytics (1500,1750,2000, 2250)
set label 1 "Aligned pairs (in thousands)"
set xlabel ""
set size 1,0.5
set origin 0.0,0.5
set bmargin 0
set lmargin 5
set tmargin 3
plot "160k.masterPairs.dat" using 1:2 title "Pairs aligned" w lp lw 3 pt 1, \
"tree.dat" using 1:3 title "Pairs generated" w lp lw 3 pt 4
set nomultiplot

