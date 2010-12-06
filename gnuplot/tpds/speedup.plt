reset
set term post color 
#set term post
set out 'speedup.eps'

set size 0.7
set datafile missing 'NaN'
set key top left
set key box
set grid
set xrange[0:9]
set yrange[0:2100] 
set xtics ("16" 1, "32" 2, "64" 3, "128" 4, "256" 5, "512" 6, "1024" 7, "2048" 8)  
set ytics (32, 128, 256, 512, 1024, 2048)  
set xlabel "Number of processors"
set ylabel "Speedup"

plot "speedup.dat" using 1:2 title "n=20K" w lp lw 3 pt 1, \
"" using 1:3 title "n=40K" w lp lw 3 pt 2, \
"" using 1:4 title "n=80K" w lp lw 3 pt 3, \
"" using 1:5 title "n=160K" w lp lw 3 pt 4, \
"" using 1:6 title "n=320K" w lp lw 3 pt 5, \
"" using 1:7 title "n=640K" w lp lw 3 pt 6, \
"" using 1:8 title "n=1,280K" w lp lw 3 pt 7,\
"" using 1:9 title "n=2,560K" w lp lw 3 pt 8,\
"" using 1:10 title "ideal" w lp lw 3 pt 9

