#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <mpi.h>
#include <sys/time.h>
#include <stdint.h>
#include "tvUtil.h"

using namespace std;

double MBPS(uint64_t long bytes, double seconds){
  return bytes / 1024.0 / 1024.0 / seconds;
}

int main(int argc, char **argv){
  MPI_Init(&argc, &argv);
  if(argc < 2){
    cout << "usage: " << argv[0] << " <input file> <rows> <colums>" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  
  int id, numProcs;
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  uint64_t rows = atoi(argv[2]);
  uint64_t cols = atoi(argv[3]);
  uint64_t myCols = cols / numProcs;
  uint64_t colStart = id * myCols;
  uint64_t colEnd = colStart + myCols;
  uint64_t offset = colStart * rows * sizeof(double);
  // for 5000-element double columns, this is 4 MB, (not MiB)
  uint64_t colInc = 100;
  
  struct timeval tStart, tEnd;
  gettimeofday(&tStart, 0);
  FILE *file = fopen(argv[1], "r");
  cout << "id " << id << " seeking to " << offset << endl;
  int status = fseeko(file, offset, SEEK_SET);
  if(status){
    perror("fseek");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  if(ftello(file) != offset || feof(file)){
    cout << "seek failed" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  double *array = (double*)malloc(rows * colInc * sizeof(double));
  if(!array){
    cout << "malloc error" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  cout << "id " << id << " reading " << colInc << " colums at a time" << endl;
  for(uint64_t col = colStart; col < colEnd; col += colInc){
    size_t fstatus = fread(array, sizeof(double), rows * colInc, file);
    if(fstatus != rows * colInc)
    {
      cout << "read failed on id " << id << endl;
      if(feof(file))
	cout << id << " end of file" << endl;
      else if(ferror(file)){
	cout << id << " " << ferror(file) << endl;
      }
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
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
    cout << "mean time: " << meanTime << "s, " 
	 << MBPS(myCols * rows * sizeof(double), meanTime) << " MBPS"
	 << " (" << MBPS(cols * rows * sizeof(double), meanTime)
	 << " MBPS aggregate)"
	 << endl << "min time:  " << minTime << "s, "
	 << MBPS(myCols * rows * sizeof(double), minTime) << " MBPS"
	 << endl << "max time:  " << maxTime << "s, "
	 << MBPS(myCols * rows * sizeof(double), maxTime) << " MBPS"
	 << endl;
  }
  MPI_Finalize();
  return 0;
}
