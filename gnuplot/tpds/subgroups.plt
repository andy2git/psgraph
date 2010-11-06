reset
set term post color 
set out 'subgroups.eps'

set grid
set multiplot
set yrange[1200:1800]
set key bottom right
set label 1 "Run time (secs)" at graph -0.1, graph 0.5 center rotate
#set ylabel "Run time (secs)"
set xlabel "Subgroup ID"
set xtics 5
set ytics (1200, 1400, 1600)
set size 1,0.5
set origin 0.0,0.0
set bmargin 3
set lmargin 5
set tmargin 0
plot "subgroups.dat.sp" using 1:2 title "With pairs redistribution" w lp lw 3 pt 1, \
"sg.dat.nosp" using 1:2 title "Without pairs redistribution" w lp lw 3 pt 4
set yrange [6000:12000]
set ytics (6000, 8000, 10000, 12000)
set label 1 "#pairs (in thousands)"
set xlabel ""
set size 1,0.5
set origin 0.0,0.5
set bmargin 0
set lmargin 5
set tmargin 3
plot "subgroups.dat.sp" using 1:($3/1000) title "Pairs aligned" w lp lw 3 pt 1, \
"sg.dat.nosp" using 1:($3/1000) title "Pairs generated" w lp lw 3 pt 4
set nomultiplot

