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

double MBPS(uint64_t bytes, double seconds){
  return bytes / 1024.0 / 1024.0 / seconds;
}

int main(int argc, char **argv){
  uint64_t readSize = 32 * 1024 * 1024;
  uint64_t readLength = readSize / sizeof(double);
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
  uint64_t totalSize = rows * cols * sizeof(double);
  uint64_t mySize = totalSize / numProcs;
  uint64_t myLength = mySize / sizeof(double);
  uint64_t offset = mySize * id;
  
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

  double *array = (double*)malloc(readSize);
  if(!array){
    cout << "malloc error" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  cout << "id " << id << " reading " << readLength << " elements at a time" << endl;
  uint64_t left = mySize;
  while(left){
    size_t fstatus = fread(array, sizeof(double), min(readLength, myLength), file);
    if(fstatus != readLength)
    {
      cout << "read failed on id " << id << endl;
      if(feof(file))
	cout << id << " end of file" << endl;
      else if(ferror(file)){
	cout << id << " " << ferror(file) << endl;
      }
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    left -= min(readSize, left);
  }

  fclose(file);
  gettimeofday(&tEnd, 0);

  double myTime = tvDouble(tEnd - tStart);
  double meanTime, maxTime, minTime;
  MPI_Reduce(&myTime, &meanTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&myTime, &minTime, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  MPI_Reduce(&myTime, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(!id){
    meanTime /= numProcs;
    cout << "mean time: " << meanTime << "s, " 
	 << MBPS(mySize, meanTime) << " MBPS"
	 << " (" << MBPS(totalSize, meanTime)
	 << " MBPS aggregate)"
	 << endl << "min time:  " << minTime << "s, "
	 << MBPS(mySize, minTime) << " MBPS"
	 << endl << "max time:  " << maxTime << "s, "
	 << MBPS(mySize, maxTime) << " MBPS"
	 << endl;
  }
  MPI_Finalize();
  return 0;
}
