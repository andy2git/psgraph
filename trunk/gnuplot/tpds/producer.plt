reset
set term post color 
set out 'producer.eps'

set grid
set multiplot
set yrange [1500:3500]
set key bottom right
set lmargin 5
set label 1 "#trees" at graph -0.1, graph 0.5 center rotate
set ytics (2000, 2500, 3000)
set xlabel "Producer ID"
set xtics 5
set size 1,0.3
set origin 0.0,0.0
set bmargin 3
set lmargin 5
set tmargin 0
plot "producer.dat" using 1:2 title "Number of processed trees" w lp lw 3 pt 1
set yrange [1000:3000]
set label 1 "#pairs (in thousands)"
#set ylabel "#pairs (in thousands)"
set ytics (1500, 1750,2000, 2250)
set xlabel " "
set size 1,0.4
set origin 0.0,0.3
set bmargin 0
set lmargin 5
set tmargin 0
plot "producer.dat" using 1:3 title "Generated pairs" w lp lw 3 pt 2
set yrange [50:80]
#set ylabel "Run time (secs)"
set label 1 "Run time (secs)"
set ytics (65, 70)
set xlabel " "
set size 1,0.3
set origin 0.0,0.7
set bmargin 0
set lmargin 5
set tmargin 3
plot "producer.dat" using 1:4 title "Pair generation time" w lp lw 3 pt 3
set nomultiplot

