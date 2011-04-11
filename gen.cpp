#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
//#include <lustre/liblustreapi.h>
//#include <lustre/lustre_user.h>
//int llapi_file_create(char *name, long stripe_size, int stripe_offset, int stripe_count, int stripe_pattern);

using namespace std;
int main(int argc, const char **argv){

  unsigned rows = 0, cols = 0;
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
  for(unsigned i = 0; i < rows * cols; i++){
    double element = drand48();
    fwrite(&element, sizeof(double), 1, file);
  }
  return fclose(file);
}
