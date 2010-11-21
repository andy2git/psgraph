reset
set term post color 
set out 'fetch.eps'

set grid
set multiplot
set yrange [0:3000]
set xrange [-10:710]
set key bottom right
set lmargin 5
set label 1 "#Saved seqs." at graph -0.1, graph 0.5 center rotate
set ytics (800, 1500, 2200) 
set xlabel "Iterations"
set size 1,0.3
set origin 0.0,0.0
set bmargin 3
set lmargin 5
set tmargin 0
plot "fetch.d" u 1:4 notitle w l lw 2
set yrange [0:1300]
set label 1 "#Fetched seqs."
set ytics (400,600,800)
set xlabel " "
set size 1,0.3
set origin 0.0,0.3
set bmargin 0
set lmargin 5
set tmargin 0
plot "fetch.d" u 1:2 notitle w l lw 2
set yrange [0:2500]
set label 1 "#Cached seqs."
set ytics (800, 1200, 1600)
set xlabel " "
set size 1,0.4
set origin 0.0,0.6
set bmargin 0
set lmargin 5
set tmargin 3
plot "fetch.d" u 1:3 notitle w l lw 2
set nomultiplot

