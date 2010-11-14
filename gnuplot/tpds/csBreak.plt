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
set boxwidth 0.2
plot newhistogram "I/O version", "csBreak.dat" using 2:xtic(1) ti 2, '' u 3 ti 3, '' u 4 ti 4, '' u 5 ti 5,\
    newhistogram "Non-blocking version", '' u 6:xtic(1) ti 6, '' u 7 ti 7, '' u 8 ti 8
