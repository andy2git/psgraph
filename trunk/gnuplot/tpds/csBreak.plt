reset
set term post color 
set out 'csBreak.eps'
set key top right

set size 1.0
set auto x
set grid
set yrange [0:32000]
set ylabel "Average run time (secs)"
set xlabel "Number of processors"
set style data histograms
set style histogram rowstacked
set style fill pattern 2
set boxwidth 0.5
set xtic rotate by -45 scale 0
plot \
newhistogram "I/O version", \
"csBreak.dat" using 2:xtic(1) title "Align time", '' u 3 title "Idle time", '' u 4 title "I/O time", '' u 5 title "Comm. time", \
newhistogram "Non-blocking version", \
 'csBreak.nb.dat' using 2:xtic(1) title "Align time", '' u 3 title "Idle time", '' u 4 title  "NB. overhead", '' u 5 title "Comm. time"
