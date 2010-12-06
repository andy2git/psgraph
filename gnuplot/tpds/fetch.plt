reset
set term post color 
set out 'fetch.eps'

set grid
set multiplot
set yrange [0:1]
set xrange [-10:730]
set key bottom right
set lmargin 5
set label 1 "#Saved seqs." at graph -0.1, graph 0.5 center rotate
set ytics (.2, .4, .6, .8) 
set xlabel "Iterations"
set size 1,0.3
set origin 0.0,0.0
set bmargin 3
set lmargin 5
set tmargin 0
plot "fetch.dat" u 1:4 notitle w l lw 2
set yrange [0:1]
set ytics (.2, .4, .6) 
set label 1 "#fetched seqs."
set xlabel " "
set size 1,0.3
set origin 0.0,0.3
set bmargin 0
set lmargin 5
set tmargin 0
plot "fetch.dat" u 1:2 notitle w l lw 2
set yrange [0:1]
set label 1 "#Cached seqs."
set ytics (.2, .4, .6, .8) 
set xlabel " "
set size 1,0.4
set origin 0.0,0.6
set bmargin 0
set lmargin 5
set tmargin 3
plot "fetch.dat" u 1:3 notitle w l lw 2
set nomultiplot

