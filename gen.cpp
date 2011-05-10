#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <errno.h>
extern "C"{
#include <lustre/liblustreapi.h>
}
#include <mpi.h>

/*! @todo pad created matrix file by system page size. 
  This helps with llapi_file_create and mmap2.
  Get page size with getpagesize() (unistd.h)
 */
//int llapi_file_create(char *name, long stripe_size, int stripe_offset, int stripe_count, int stripe_pattern);

using namespace std;
int main(int argc, char **argv){
  int status;
  uint64_t readSize = 32 * 1024 * 1024;
  uint64_t readLength = readSize / sizeof(double);
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

  #ifdef haveLustre
  if(!id){
    status = unlink(argv[3]);
    if(status && errno != ENOENT){
      perror("unlink");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    sync();

    status = llapi_file_create(argv[3], 
  			       (readSize / getpagesize()) * 
  			       getpagesize(),
  			       -1,
  			       0,
  			       0);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  #endif

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
  uint64_t totalSize = rows * cols * sizeof(double);
  uint64_t mySize = totalSize / numProcs;
  uint64_t offset = mySize * id;
  //cout << "seeking to " << offset << endl;

  status = fseeko(file, offset, SEEK_SET);
  if(status){
    perror("fseek");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  if(ftello(file) != offset || feof(file)){
    cout << "seek failed" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

#ifndef _SPEED
#warning generating a nonuniform distribution, inducing slowness
  cout << "generating a nonuniform distribution, inducing slowness" << endl;
#endif
  cout << "id " << id << " writing " << readLength << " elements at a time" << endl;

  // assume column-major
  double *array = (double*)malloc(readSize);
  if(!array){
    cout << "malloc error" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  uint64_t left = mySize;
  while(left){
    for(uint64_t index = 0; index < readLength; index++){
      array[index] = drand48();
#ifndef _SPEED
      if(array[index] < .4249)
	array[index] = 2;
      else if(array[index] < .4249 + .0329)
	array[index] = 1;
      else if(array[index] < .4249 + .0329 + .459)
	array[index] = 0;
#endif
    }
    fwrite(array, sizeof(double), readLength, file);
    left -= min(readSize, left);
  }
  fsync(fileno(file));
  fclose(file);
  MPI_Finalize();
  return 0;
}
