#!/bin/bash
./gen 5000 2500000 test.dat
#5000 x 2500000, 100 GB
sync
mpirun -n 10 ./read test.dat 5000 2500000

