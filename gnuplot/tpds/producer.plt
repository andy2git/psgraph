reset
set term post color 
set out 'producer.eps'

set grid
set multiplot
set yrange [900:2000]
set key bottom right
set lmargin 5
set label 1 "#trees" at graph -0.1, graph 0.5 center rotate
set ytics (1000,1500)
set xlabel "Producer ID"
set xtics 5
set size 1,0.3
set origin 0.0,0.0
set bmargin 3
set lmargin 5
set tmargin 0
plot "producer.dat" using 1:2 title "Number of processed trees" w lp lw 3 pt 1
set yrange [3000:7000]
set label 1 "#pairs (in thousands)"
#set ylabel "#pairs (in thousands)"
set ytics (4000, 4500, 5000)
set xlabel " "
set size 1,0.4
set origin 0.0,0.3
set bmargin 0
set lmargin 5
set tmargin 0
plot "producer.dat" using 1:($3/1000) title "Generated pairs" w lp lw 3 pt 2
set yrange [380:400]
#set ylabel "Run time (secs)"
set label 1 "Run time (secs)"
set ytics (385,395)
set xlabel " "
set size 1,0.3
set origin 0.0,0.7
set bmargin 0
set lmargin 5
set tmargin 3
plot "producer.dat" using 1:4 title "Pair generation time" w lp lw 3 pt 3
set nomultiplot

