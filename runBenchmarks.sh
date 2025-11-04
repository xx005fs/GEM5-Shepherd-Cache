rm -rf m5out*
rm ./configs/common/Caches.py
cp cache_lru.py ./configs/common/
mv ./configs/common/cache_lru.py ./configs/common/Caches.py
echo Running benchmarks for LRU cache replacement policy
./build/ECE565-ARM/gem5.fast --outdir=m5out_lru_bzip2 --stats-file=bzip2_stats.txt configs/spec/spec_se.py -b bzip2 --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_lru_gcc --stats-file=gcc_stats.txt configs/spec/spec_se.py -b gcc --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_lru_milc --stats-file=milc_stats.txt configs/spec/spec_se.py -b milc --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_lru_sjeng --stats-file=sjeng_stats.txt configs/spec/spec_se.py -b sjeng --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_lru_namd --stats-file=namd_stats.txt configs/spec/spec_se.py -b namd --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_lru_astar --stats-file=astar_stats.txt configs/spec/spec_se.py -b astar --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_lru_lbm --stats-file=lbm.txt configs/spec/spec_se.py -b lbm --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_lru_leslie3d --stats-file=leslie3d_stats.txt configs/spec/spec_se.py -b leslie3d --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_lru_cactusADM --stats-file=cactusADM_stats.txt configs/spec/spec_se.py -b cactusADM --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
wait
echo Finished running LRU cache replacement policy benchmarks

rm ./configs/common/Caches.py
cp cache_sc.py ./configs/common/
mv ./configs/common/cache_sc.py ./configs/common/Caches.py
echo Running benchmarks for shepherd cache replacement policy
./build/ECE565-ARM/gem5.fast --outdir=m5out_sc_bzip2 --stats-file=bzip2_stats.txt configs/spec/spec_se.py -b bzip2 --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_sc_gcc --stats-file=gcc_stats.txt configs/spec/spec_se.py -b gcc --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_sc_milc --stats-file=milc_stats.txt configs/spec/spec_se.py -b milc --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_sc_sjeng --stats-file=sjeng_stats.txt configs/spec/spec_se.py -b sjeng --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_sc_namd --stats-file=namd_stats.txt configs/spec/spec_se.py -b namd --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_sc_astar --stats-file=astar_stats.txt configs/spec/spec_se.py -b astar --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_sc_lbm --stats-file=lbm.txt configs/spec/spec_se.py -b lbm --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_sc_leslie3d --stats-file=leslie3d_stats.txt configs/spec/spec_se.py -b leslie3d --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
./build/ECE565-ARM/gem5.fast --outdir=m5out_sc_cactusADM --stats-file=cactusADM_stats.txt configs/spec/spec_se.py -b cactusADM --caches --l1d_size=16kB --l1i_size=16kB --l2cache --l2_size=512kB --l2_assoc=16 --maxinsts=500000000 -W=200000000 &
wait
echo Finished running shepherd cache replacement policy benchmarks
./displayResults.sh

# Benchmark List
# bzip2, gcc, mcf(?), milc, cactusADM, leslie3d, namd, hmmer(?), sjeng, lbm, astar, omnetpp(?)