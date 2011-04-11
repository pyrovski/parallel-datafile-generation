#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <lustre/liblustreapi.h>
#include <lustre/lustre_user.h>
#include <mpi.h>

/*! @todo pad created matrix file by system page size. 
  This helps with llapi_file_create and mmap2.
  Get page size with getpagesize() (unistd.h)
 */
//int llapi_file_create(char *name, long stripe_size, int stripe_offset, int stripe_count, int stripe_pattern);

using namespace std;
int main(int argc, char **argv){
  MPI_Init(&argc, &argv);

  uint64_t rows = 0, cols = 0;
  FILE *file;
  
  if(argc < 4){
    cout << "usage: " << argv[0] << " <rows> <columns> <output file>" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  
  int id, numProcs;
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

  file = fopen(argv[3], "w");
  if(!file){
    perror("open");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  if(ftruncate(fileno(file), rows * cols * sizeof(double))){
    perror("ftruncate");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  long flags = fcntl(fileno(file), F_GETFL);
  if(fcntl(fileno(file), F_SETFL, flags | O_NONBLOCK)){
    perror("fcntl");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  rows = atoi(argv[1]);
  cols = atoi(argv[2]);
  unsigned myCols = cols / numProcs;
  unsigned colStart = id * myCols;
  unsigned colEnd = colStart + myCols;
  uint64_t offset = colStart * rows * sizeof(double);
  //cout << "seeking to " << offset << endl;

  int status = fseeko(file, offset, SEEK_SET);
  if(status){
    perror("fseek");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  if(ftello(file) != offset || feof(file)){
    cout << "seek failed" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  //cout << "id " << id << " writing " << myCols << " cols." << endl;

  // assume column-major
  double *array = (double*)malloc(rows * sizeof(double));
  for(uint64_t col = colStart; col < colEnd; col++){
    for(uint64_t row = 0; row < rows; row++){
      array[row] = drand48();
    }
    fwrite(array, sizeof(double), rows, file);
  }
  fsync(fileno(file));
  fclose(file);
  MPI_Finalize();
  return 0;
}
