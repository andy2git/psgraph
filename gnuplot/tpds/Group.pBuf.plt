reset
set term post
set out 'Group.pBuf.eps'

set multiplot
set grid
set size 0.5,1
set origin 0.0,0.0
set lmargin 5 
set rmargin 0
set xrange[0:600]
set xtics 0,200
set yrange[0:100]
set ytics 0,10
set ylabel "Pairs buffered on master (in thousands)"
set xlabel " "
plot "Group.pBuf.dat" u 1:2 notitle w p pt 1
set origin 0.5,0.0
set format y ""
set lmargin 0
set rmargin 1
set xrange[1000:10000]
set xtics 600,2000
set yrange[0:100]
set ytics 0,10
set ylabel ""
#set xlabel "Iterations" -20,0
plot "Group.pBuf.dat" u 1:2 notitle w points
set nomultiplot


