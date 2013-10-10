#Note: the #PBS designator is "magic"; it is NOT a comment!
# set the requested resources
#PBS -l mem=150000mb,nodes=2:ppn=64,walltime=23:20:00
#PBS -V


cd $PBS_O_WORKDIR

mpirun /home/cwu2/psgraph/src/pclust -f /home/cwu2/psgraph/src/suffixtree/ -s /mnt/pvfs2/cwu2/bmcrun/data/input.seqs/pf_2M.fa -n 2004241 -m 1 -c /home/cwu2/pclust.SR/cfg.conf -g 64 -p 16 -o /mnt/pvfs2/cwu2/hicomb/ > /mnt/pvfs2/cwu2/hicomb/tmp.log

