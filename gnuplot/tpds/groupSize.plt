reset
#set term post color
set term post
set out 'groupSize.eps'
set key top right

set style line 101 lc rgb "grey10"
set style line 102 lc rgb "grey70"
set style line 103 lc rgb "grey20"
set style line 104 lc rgb "grey80" 
set size 0.7
set auto x
set grid
set yrange [0:2500]
set ylabel "Run time (secs)"
set xlabel "Group size"
set style data histograms
set style histogram rowstacked
set style fill pattern 1
set boxwidth 0.5
plot "groupSize.dat" using 3:xtic(1) title "Idle time", '' u 2 title "Non-idle. time"
