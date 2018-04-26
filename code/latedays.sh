#!/bin/bash

# Limit execution time to 20 minutes
#PBS -lwalltime=0:30:00
# Allocate all available CPUs on a single node
# PBS -l nodes=1:ppn=24

# Go to the directory from which you submitted your job
cd $PBS_O_WORKDIR

# Execute performance evaluation program
./test-seq
./test-omp
