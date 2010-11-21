reset
set term post color
set out 'msBreak.eps'
set key top right

set style line 101 lc rgb "grey10"
set style line 102 lc rgb "grey70"
set style line 103 lc rgb "grey20"
set style line 104 lc rgb "grey80" 
set size 0.7
set auto x
set grid
set yrange [0:25000]
set ylabel "Average run time (secs)"
set xlabel "Number of processors"
set style data histograms
set style histogram rowstacked
set style fill pattern 2
set boxwidth 0.5
plot "msBreak.dat" using 2:xtic(1) title "Idle time", '' u 3 title "Comm. time", '' u 4 title "Comp. time"
