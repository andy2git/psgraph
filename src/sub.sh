#Note: the #PBS designator is "magic"; it is NOT a comment!
# set the requested resources
#PBS -l mem=12000mb,nodes=1:ppn=14,walltime=00:20:00
#PBS -V



cd $PBS_O_WORKDIR

mpirun /home/cwu2/pclust.SR/pclust -f /home/cwu2/pclust/data/20k -s /home/cwu2/pclust/pf_20k.fa.prep.nr -n 20000 -c /home/cwu2/pclust.SR/cfg.conf -g 14 -p 4 -o /home/cwu2/pclust/todel/ > tmp.log

