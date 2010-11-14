reset
set term post color 
set out 'csBreak.eps'
set key top right

set size 0.7
set auto x
set grid
set yrange [0:32000]
set ylabel "Average run time (secs)"
set xlabel "Number of processors"
set style data histograms
set style histogram rowstacked
set style fill pattern 1
set boxwidth 0.5
set xtic rotate by -45 scale 0
plot "csBreak.dat" using 2:xtic(1) title "Align time", '' u 3 title "Idle time", '' u 4 title "I/O time", '' u 5 title  "A2A overhead", '' u 6 title "Comm. time"
