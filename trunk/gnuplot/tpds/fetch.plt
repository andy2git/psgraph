reset
set term post
set out 'fetch.eps'

set grid
set multiplot
set yrange [0:100]
set format y "%g %%"
set xrange [-10:730]
set key bottom right
set lmargin 5
set label 1 "Dynamic hit ratio" at graph -0.1, graph 0.5 center rotate
set ytics (20, 40, 60, 80) 
set xlabel "Iterations"
set size 1,0.3
set origin 0.0,0.0
set bmargin 3
set lmargin 5
set tmargin 0
plot "fetch.dat" u 1:($4*100) notitle w l lw 2
set yrange [0:100]
set ytics (20, 40, 60) 
set label 1 "Fetched ratio"
set xlabel " "
set size 1,0.3
set origin 0.0,0.3
set bmargin 0
set lmargin 5
set tmargin 0
plot "fetch.dat" u 1:($2*100) notitle w l lw 2
set yrange [0:100]
set label 1 "Static hit ratio"
set ytics (20, 40, 60, 80) 
set xlabel " "
set size 1,0.4
set origin 0.0,0.6
set bmargin 0
set lmargin 5
set tmargin 3
plot "fetch.dat" u 1:($3*100) notitle w l lw 2
set nomultiplot

