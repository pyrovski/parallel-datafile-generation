#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <mpi.h>

using namespace std;
int main(int argc, char **argv){
  if(argc < 2){
    cout << "usage: " << argv[0] << " <input file> <rows> <colums>" << endl;
    return 1;
  }

  MPI_Init(&argc, &argv);
  MPI_Finalize();

  return 0;
}
