#!/bin/bash

SIZE=30 #Depolarization probability
N=20  # Number of times to run the benchmark
MIN_PROB=0.001
MAX_PROB=0.051
STEP_PROB=0.005
INITIAL_SEED=123456789
OUTFILE="imperfect_rotated_planar_fixed_size.dat" #File for the time output 

for pr in $(seq $MIN_PROB $STEP_PROB $MAX_PROB); do 
    start=$(date +%s)  # Capture the starting Unix timestamp
    echo "Running pymatching benchmark for prob=$pr"
   SUM=0  # Variable to store the sum of times

   for ((i=1; i<=N; i++)); do
       NEW_SEED=$(awk "BEGIN {print $INITIAL_SEED + $i*100000}")
       time=$(python3 /home/ubuntu/benchmark/benchmark.py rotated_planar $SIZE imperfect $pr pymatching $NEW_SEED | tail -n1 | cut -d ':' -f2 | tr -d ' ')
       SUM=$(awk "BEGIN {print $SUM + $time}")
   done
   
   average=$(awk "BEGIN {print $SUM / $N}")
   echo "pymatching",$SIZE,$pr,$average,"rotated_planar","imperfect" >> $OUTFILE

   echo "Running fusion-blossom benchmark for prob=$pr"
   SUM=0  # Variable to store the sum of times

   for ((i=1; i<=N; i++)); do
       NEW_SEED=$(awk "BEGIN {print $INITIAL_SEED + $i*100000}")
       time=$(python3 /home/ubuntu/benchmark/benchmark.py rotated_planar $SIZE imperfect $pr fusion-blossom $NEW_SEED | tail -n1 | cut -d ':' -f2 | tr -d ' ')
       SUM=$(awk "BEGIN {print $SUM + $time}")
   done
   
   average=$(awk "BEGIN {print $SUM / $N}")
   echo "fusion-blossom",$SIZE,$pr,$average,"rotated_planar","imperfect" >> $OUTFILE

   echo "Running plaquette-unionfind benchmark for prob=$pr"
   SUM=0  # Variable to store the sum of times
  
   for ((i=1; i<=N; i++)); do
       NEW_SEED=$(awk "BEGIN {print $INITIAL_SEED + $i*100000}")
       time=$(python3 /home/ubuntu/benchmark/benchmark.py rotated_planar $SIZE imperfect $pr plaquette-unionfind $NEW_SEED | tail -n1 | cut -d ':' -f2 | tr -d ' ')
       SUM=$(awk "BEGIN {print $SUM + $time}")
   done
   
   average=$(awk "BEGIN {print $SUM / $N}")
   echo "plaquette-unionfind",$SIZE,$pr,$average,"rotated_planar","imperfect" >> $OUTFILE

   end=$(date +%s)  # Capture the ending Unix timestamp
   elapsed=$((end - start))  # Calculate the elapsed time in seconds

   echo "Elapsed time: $elapsed seconds"   
done
