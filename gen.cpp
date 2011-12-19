#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#ifdef haveLustre
extern "C"{
#include <lustre/liblustreapi.h>
}
#endif
#include <mpi.h>

/*! @todo pad created matrix file by system page size. 
  This helps with llapi_file_create and mmap2.
  Get page size with getpagesize() (unistd.h)
*/
//int llapi_file_create(char *name, long stripe_size, int stripe_offset, int stripe_count, int stripe_pattern);

using namespace std;
int id, numProcs;


template <class T> int gen(FILE *file, uint64_t rows, uint64_t cols, const char *typeName){
  //! @todo get output filename and dimensions from command line with getopt

  int status;

  if(ftruncate(fileno(file), rows * cols * sizeof(T))){
    perror("ftruncate");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  long flags = fcntl(fileno(file), F_GETFL);
  if(fcntl(fileno(file), F_SETFL, flags | O_NONBLOCK)){
    perror("fcntl");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  uint64_t readSize = 32 * 1024 * 1024;
  //This is the "count" in fwrite, number of elements to be written-- same for float and double dataset
  uint64_t readLength = readSize / sizeof(T);

  uint64_t totalSize = rows * cols * sizeof(T);
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
  cout << "id " << id << " writing " << readLength << " " << typeName 
       << " elements at a time" << endl;

  // assume column-major
  T *array = (T*)malloc(readSize);
  if(!array){
    cout << "malloc error" << endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  //! @todo set seed from argument
  srand48(0);

  uint64_t left = mySize/sizeof(T);
  while(left){
    for(uint64_t index = 0; index < min(readLength, left); index++){
      array[index] = drand48();

#ifndef _SPEED
      if(array[index] < .4249){
	array[index] = 2;
      }
      else if(array[index] < .4249 + .0329){
	array[index] = 1;
      }
      else if(array[index] < .4249 + .0329 + .459){
	array[index] = 0;
      }
#endif
    }
    //Write out to both the files  
    fwrite(array, sizeof(T), min(readLength, left), file);
    
    left -= min(readLength, left);
  }
  
  fsync(fileno(file));


  fclose(file);
  MPI_Finalize();
  return 0;

}

void usage(char **argv){
      cerr << "usage: " << argv[0] 
	   << "[-f (single) or -d (double)] -r <rows> -c <columns> -o <output file>" 
	   << endl;

}
int main(int argc, char **argv){
  
  int  option, type = 'd';
  uint64_t rows = 0, cols = 0;
  char argFilename[256] = "";
  FILE *file;

  char filename[256];

  MPI_Init(&argc, &argv);
  
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

  while((option = getopt(argc, argv, "fd:r:c:o:")) != -1){
    switch(option){
    case 'r':
      rows = strtoull(optarg, 0, 0);
      break;
    case 'c':
      cols = strtoull(optarg, 0, 0);
      break;
    case 'o':
      strncpy(argFilename, optarg, 256);
      argFilename[255] = 0;
      break;
    case 'f':
      type = 'f';
      break;
    case 'd':
      type = 'd';
      break;
    default:
      cerr << "invalid option" << endl;
      usage(argv);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  if(!rows || !cols || !argFilename){
    usage(argv);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  switch(type){
  case 'f':
    sprintf(filename,"%s.%s", argFilename, "float");
    break;
  case 'd':
    sprintf(filename,"%s.%s", argFilename, "double");
    break;
  }
    

#ifdef haveLustre
  if(!id){
#error fixme
    status = unlink(filename);
    if(status && errno != ENOENT){
      perror("unlink");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    sync();

    status = llapi_file_create(filename, 
  			       (readSize / getpagesize()) * 
  			       getpagesize(),
  			       -1,
  			       0,
  			       0);

  }
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  file = fopen(filename, "w");

  if(!file){
    perror("open");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  if(!id){
    cout << "selected ";
    
    switch(type){
    case 'f':
      cout << "float type" << endl;
      return gen<float>(file, rows, cols, "float");
    case 'd':
      cout << "double type" << endl;
      return gen<double>(file, rows, cols, "double");
    default:
      cerr << "invalid type specifier: " << type << endl;
      return -1;
    }
  }
}
