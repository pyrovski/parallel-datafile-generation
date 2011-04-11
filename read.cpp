#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <mpi.h>
#include <sys/time.h>

using namespace std;
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
  FILE *file = fopen(argv[1], "r");
  unsigned rows = atoi(argv[2]);
  unsigned cols = atoi(argv[3]);
  unsigned myCols = cols / numProcs;
  unsigned colStart = id * myCols;
  unsigned colEnd = colStart + myCols;
  long offset = id * colStart * rows * sizeof(double);
  int status = fseek(file, offset, SEEK_SET);
  
  // just read one column at a time (assume column-major)
  struct timeval tStart, tEnd;
  gettimeofday(&tStart, 0);
  double *array = (double*)malloc(rows * sizeof(double));
  for(unsigned col = colStart; col < colEnd; col++){
    fread(array, sizeof(double), rows, file);
  }
  
  gettimeofday(&tEnd, 0);

  MPI_Finalize();
  return 0;
}
