#include<unistd.h>
#include<stdio.h>
#include <mpi.h>
int main(int argc, char **argv)
{
   MPI_Init(&argc, &argv);
   int retVal = 0;
   retVal = getpagesize();
   if( retVal >= 0)
   {
      printf("getpagesize passed");
      printf("\n retVal  = %d ", retVal);
   }
   else
   {
   printf("Failed");
	MPI_Abort(MPI_COMM_WORLD, 1);
   }
MPI_Finalize();
return 0;

}
