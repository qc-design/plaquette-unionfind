#!/bin/bash

PROB=$1 #Depolarization probability
N=10  # Number of times to run the benchmark
MIN_SIZE=5
MAX_SIZE=40
INITIAL_SEED=123456789
OUTFILE="imperfect_rotated_planar_${PROB}.dat" #File for the time output 

for s in $(seq $MIN_SIZE $MAX_SIZE); do 
    start=$(date +%s)  # Capture the starting Unix timestamp
    echo "Running pymatching benchmark for s=$s"
   SUM=0  # Variable to store the sum of times

   for ((i=1; i<=N; i++)); do
       NEW_SEED=$(awk "BEGIN {print $INITIAL_SEED + $i*100000}")
       time=$(python3 /home/ubuntu/benchmark/benchmark.py planar $s imperfect $PROB pymatching $NEW_SEED | tail -n1 | cut -d ':' -f2 | tr -d ' ')
       SUM=$(awk "BEGIN {print $SUM + $time}")
   done
   
   average=$(awk "BEGIN {print $SUM / $N}")
   echo "pymatching",$s,$PROB,$average,"planar","imperfect" >> $OUTFILE

   echo "Running fusion-blossom benchmark for s=$s"
   SUM=0  # Variable to store the sum of times

   for ((i=1; i<=N; i++)); do
       NEW_SEED=$(awk "BEGIN {print $INITIAL_SEED + $i*100000}")
       time=$(python3 /home/ubuntu/benchmark/benchmark.py planar $s imperfect $PROB fusion-blossom $NEW_SEED | tail -n1 | cut -d ':' -f2 | tr -d ' ')
       SUM=$(awk "BEGIN {print $SUM + $time}")
   done
   
   average=$(awk "BEGIN {print $SUM / $N}")
   echo "fusion-blossom",$s,$PROB,$average,"planar","imperfect" >> $OUTFILE

   echo "Running plaquette-unionfind benchmark for s=$s"
   SUM=0  # Variable to store the sum of times
  
   for ((i=1; i<=N; i++)); do
       NEW_SEED=$(awk "BEGIN {print $INITIAL_SEED + $i*100000}")
       time=$(python3 /home/ubuntu/benchmark/benchmark.py planar $s imperfect $PROB plaquette-unionfind $NEW_SEED | tail -n1 | cut -d ':' -f2 | tr -d ' ')
       SUM=$(awk "BEGIN {print $SUM + $time}")
   done
   
   average=$(awk "BEGIN {print $SUM / $N}")
   echo "plaquette-unionfind",$s,$PROB,$average,"planar","imperfect" >> $OUTFILE

   end=$(date +%s)  # Capture the ending Unix timestamp
   elapsed=$((end - start))  # Calculate the elapsed time in seconds

   echo "Elapsed time: $elapsed seconds"   
done
