reset
set term post
set out 'producer.eps'

set grid
set multiplot
set key bottom right
set xlabel "Producer ID"
set xtics 10
set lmargin 5
set yrange [3000:7000]
set label 1 "#pairs (in thousands)" at graph -0.1, graph 0.5 center rotate
set ytics (4000, 4500, 5000)
set size 1,0.5
set origin 0.0,0.0
set bmargin 3
set lmargin 5
set tmargin 0
plot "producer.dat" using 1:($3/1000) title "Generated pairs" w lp lw 3 pt 2
set yrange [300:450]
set label 1 "Run time (secs)"
set ytics (380,400)
set xlabel " "
set size 1,0.5
set origin 0.0,0.5
set bmargin 0
set lmargin 5
set tmargin 3
plot "producer.dat" using 1:4 title "Pair generation time" w lp lw 3 pt 3
set nomultiplot

