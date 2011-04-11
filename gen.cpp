#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <lustre/liblustreapi.h>
#include <lustre/lustre_user.h>

/*! @todo pad created matrix file by system page size. 
  This helps with llapi_file_create and mmap2.
  Get page size with getpagesize() (unistd.h)
 */
//int llapi_file_create(char *name, long stripe_size, int stripe_offset, int stripe_count, int stripe_pattern);

using namespace std;
int main(int argc, const char **argv){

  uint64_t rows = 0, cols = 0;
  FILE *file;
  
  if(argc < 4){
    cout << "usage: " << argv[0] << " <rows> <columns> <output file>" << endl;
    return 1;
  }
  
  rows = atoi(argv[1]);
  cols = atoi(argv[2]);
  file = fopen(argv[3], "w");
  if(!file){
    cout << "open failed" << endl;
    return 1;
  }
  if(ftruncate(fileno(file), rows * cols * sizeof(double))){
    cout << "truncate failed" << endl;
    return 1;
  }
  long flags = fcntl(fileno(file), F_GETFL);
  if(fcntl(fileno(file), F_SETFL, flags | O_NONBLOCK)){
    cout << "fcntl failed" << endl;
    return 1;
  }
  // assume column-major
  double *array = (double*)malloc(rows * sizeof(double));
  for(uint64_t col = 0; col < cols; col++){
    for(uint64_t row = 0; row < rows; row++){
      array[row] = drand48();
    }
    fwrite(array, sizeof(double), rows, file);
  }
  fsync(fileno(file));
  return fclose(file);
}
