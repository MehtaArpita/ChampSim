#!/bin/bash

while read benchmark; do 
    echo "Running $benchmark for Hawkeye with  warmup and  simulation instructions"
    bin/champsim --warmup_instructions 200000000 --simulation_instructions 500000000 ./$benchmark > results_$benchmark.log
    #bin/champsim --warmup_instructions 20 --simulation_instructions 500 ./$benchmark > results_$benchmark.log
done < benchmarks.txt
