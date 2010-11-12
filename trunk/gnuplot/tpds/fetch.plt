reset
set term post color 
set out 'fetch.eps'

set grid
set multiplot
set yrange [0:1300]
set xrange [-10:710]
set key bottom right
set lmargin 5
set label 1 "#fetched seqs." at graph -0.2, graph 0.5 center rotate
set ytics (600)
set xlabel "Iterations"
set size 1,0.25
set origin 0.0,0.0
set bmargin 3
set lmargin 5
set tmargin 0
plot "fetch.d" u 1:2 notitle w l lw 2
set yrange [0:600]
set label 1 "#req. bytes (KB)."
set ytics (300)
set xlabel " "
set size 1,0.25
set origin 0.0,0.25
set bmargin 0
set lmargin 5
set tmargin 0
plot "fetch.d" u 1:5 notitle w l lw 2
set yrange [0:2500]
set label 1 "#cached seqs."
set ytics (1200)
set xlabel " "
set size 1,0.25
set origin 0.0,0.5
set bmargin 0
set lmargin 5
set tmargin 0
plot "fetch.d" u 1:3 notitle w l lw 2
set yrange [0:3000]
set label 1 "#saved seqs."
set ytics (1500)
set xlabel " "
set size 1,0.25
set origin 0.0,0.75
set bmargin 0
set lmargin 5
set tmargin 3
plot "fetch.d" u 1:4 notitle w l lw 2
set nomultiplot

