#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
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
  if(!file)
    return 1;
  for(uint64_t i = 0; i < rows * cols; i++){
    double element = drand48();
    fwrite(&element, sizeof(double), 1, file);
  }
  return fclose(file);
}
