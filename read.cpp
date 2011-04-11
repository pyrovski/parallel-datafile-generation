#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <mpi.h>

using namespace std;
int main(int argc, char **argv){
  int retVal;
  if(argc < 2){
    cout << "usage: " << argv[0] << " <input file> <rows> <colums>" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  MPI_Init(&argc, &argv);
  
  int id, numProcs;
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  FILE *file = fopen(argv[1], "r");
  unsigned rows = atoi(argv[2]);
  unsigned cols = atoi(argv[3]);
  long offset = id * rows * cols * sizeof(double);
  int status = fseek(file, offset, SEEK_SET);

  MPI_Finalize();
  return 0;
}
