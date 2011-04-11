#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <mpi.h>
#include <sys/time.h>
#include "tvUtil.h"

using namespace std;

double MBPS(unsigned long bytes, double seconds){
  return bytes / 1024.0 / 1024.0 / seconds;
}

int main(int argc, char **argv){
  int retVal;
  MPI_Init(&argc, &argv);
  if(argc < 2){
    cout << "usage: " << argv[0] << " <input file> <rows> <colums>" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  
  int id, numProcs;
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  unsigned rows = atoi(argv[2]);
  unsigned cols = atoi(argv[3]);
  unsigned myCols = cols / numProcs;
  unsigned colStart = id * myCols;
  unsigned colEnd = colStart + myCols;
  long offset = id * colStart * rows * sizeof(double);
  
  // just read one column at a time (assume column-major)
  struct timeval tStart, tEnd;
  gettimeofday(&tStart, 0);
  FILE *file = fopen(argv[1], "r");
  //cout << "seeking to " << offset << endl;
  int status = fseek(file, offset, SEEK_SET);
  double *array = (double*)malloc(rows * sizeof(double));
  for(unsigned col = colStart; col < colEnd; col++){
    fread(array, sizeof(double), rows, file);
  }
  fclose(file);
  gettimeofday(&tEnd, 0);

  double myTime = tvDouble(tEnd - tStart);
  double meanTime, maxTime, minTime;
  MPI_Reduce(&myTime, &meanTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  meanTime /= numProcs;
  MPI_Reduce(&myTime, &minTime, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  MPI_Reduce(&myTime, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(!id){
    cout << "mean: " << meanTime << "s, " 
	 << MBPS(myCols * rows * sizeof(double), meanTime) << " MBPS"
	 << endl << "min : " << minTime << "s, "
	 << MBPS(myCols * rows * sizeof(double), minTime) << " MBPS"
	 << endl << "max : " << maxTime << "s, "
	 << MBPS(myCols * rows * sizeof(double), maxTime) << " MBPS"
	 << endl;
  }
  MPI_Finalize();
  return 0;
}
