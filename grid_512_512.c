#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TAG 0
#define ETA 0.0002
#define RHO 0.5
#define G 0.75
#define SIZE 8
#define STRIKEX SIZE/2
#define STRIKEY SIZE/2


void exchange(int sendingProcess, int receiveProcess, float* sendingData,float* receiveBuffer,int size){
  // Request for non-blocking communication
  MPI_Request request;
  MPI_Status status;
  MPI_Sendrecv(sendingData,size,MPI_FLOAT,receiveProcess,TAG,receiveBuffer,size,MPI_FLOAT,receiveProcess,TAG,MPI_COMM_WORLD,&status);
}


void setValue(float value,float * memory, int size){
  for(int i =0;i<size;i++){
    *(memory + i)+=value;
  }
}

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    //get the number of iterations
    int *grid;
    char *ptr;
    long iterations;

    if(argc != 2){
      printf("Not enough arguments\n");
      exit(0);
    }

    iterations = strtol(argv[1], &ptr, 10);
    if(iterations <= 0){
        printf("Invalid input, please input a positive integer for the number of iterations\n");
        exit(0);
    }

    // Get the number of processes
    int numProcess;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcess);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    //calculating how many rows each process is responsible for
    int workEach = SIZE / numProcess;


    //printf("WorkEach: %d\n",workEach);

    //once we know how much each is responsible for, we use the rank
    //to determine the row for each process
    int startingRow = workEach * rank;

    //initializing the buffers in each process for their section
    float *currentValues = (float *)calloc(workEach * SIZE,sizeof(float));
    float *prevValues    = (float * )calloc(workEach * SIZE,sizeof(float));
    float *prevprevValues= (float *)calloc(workEach * SIZE,sizeof(float));

    //printf("WorkEach%d,Size%d\n",workEach,SIZE);

    //row below and row above buffers for interior
    float *rowBelow = (float *)calloc(SIZE,sizeof(float));
    float *rowAbove = (float *)calloc(SIZE,sizeof(float));

    //strike the drum but only in the process that really contains the center
    int bottomVal = workEach * rank;
    int topVal = workEach * rank + workEach;
    if( STRIKEX >= bottomVal && STRIKEY < topVal){
      *(currentValues + SIZE/2) = 1;
      //printf("StrikeX:%d, StrikeY:%d, Rank: %d\n",STRIKEX,STRIKEY,rank);
      //printf("Current values:%f",*(currentValues + SIZE/2));

    }

    for(int i =0;i<iterations;i++){
      memcpy(prevprevValues,prevValues,workEach * SIZE * sizeof(float));
      memcpy(prevValues,currentValues ,workEach * SIZE * sizeof(float));

      if(rank-1>=0){
        exchange(rank,rank-1,prevValues,rowAbove,SIZE);
      }
      //need to do an exchange with the bottom process
      if(rank+1<numProcess){
        exchange(rank,rank+1,prevValues + (workEach * SIZE)-SIZE,rowBelow,SIZE);
      }

      //we don't update the outside row if it is on the edges
      int startRow=0;
      if(rank == 0){
        startRow +=1;
      }

      int endRow = workEach;
      if(rank == numProcess-1){
        endRow -=1;
      }

      //updating center interior
      while(startRow<endRow){
        for(int column = 1; column<SIZE-1;column++){
          int leftIndex = startRow * SIZE + column - 1;
          int rightIndex = startRow * SIZE + column +1;
          int bottomIndex = (startRow+1) * SIZE + column;
          float *bottomPtr = prevValues;
          if(bottomIndex>workEach*SIZE){
            bottomPtr = rowBelow;
            bottomIndex = column;
          }

          // 00 01 02 03 04 05 06 07 RNK 1
          // 08 09 10 11 12 13 14 15
          // 16 17 18 19 20 21 22 23
          // 24 25 26 27 28 29 30 31
          //--------------------------
          // 32 33 34 35 36 37 38 39
          // 40 41 42 43 44 45 46 47
          // 48 49 50 51 52 53 54 55
          // 56 57 58 59 60 61 62 63

          int topIndex = (startRow-1) * SIZE + column;
          float*topPtr = prevValues;
          if(topIndex<0){
            topPtr = rowAbove;
            topIndex = column;
          }
          int currentIndex = (startRow) * SIZE + column;
          //printf("Left:(%d,%.2f) Right:(%d,%.2f) Bottom:(%d,%.2f) Top:(%d,%.2f) Current:(%d,%.2f)\n",leftIndex, *(prevValues+leftIndex),rightIndex,*(prevValues+rightIndex), bottomIndex,*(bottomPtr+bottomIndex),topIndex,*(topPtr+topIndex),currentIndex,*(prevValues+currentIndex));


          float leftValue = *(prevValues+leftIndex);
          float rightValue = *(prevValues+rightIndex);
          float bottomValue = *(bottomPtr+bottomIndex);
          float topValue = *(topPtr+topIndex);
          float prevValue = *(prevValues+currentIndex);
          float prevPrevValue = *(prevprevValues+currentIndex);
          float newValue = RHO * (leftValue + rightValue + bottomValue + topValue-4 * prevValue) + 2 * prevValue - (1-ETA) * prevPrevValue;
          *(currentValues+currentIndex)=newValue;
      }

        startRow++;
      }

      //completed the interior now we have to update the sides
      MPI_Barrier(MPI_COMM_WORLD);

      //reupdate the top and bottom
      if(rank-1>=0){
        exchange(rank,rank-1,prevValues,rowAbove,SIZE);
      }
      if(rank+1<numProcess){
        exchange(rank,rank+1,prevValues + (workEach * SIZE)-SIZE,rowBelow,SIZE);
      }

      startRow=0;
      if(rank == 0){
        startRow +=1;
      }

      endRow = workEach;
      if(rank == numProcess-1){
        endRow -=1;
      }

      //go through left side
      for(int i = startRow; i < endRow;i++){
        //printf("Rank %d Left Side Update %d\n",rank,SIZE*i);
        *(currentValues + (SIZE * i)) = G * (*(prevValues + (SIZE * i) + 1));
      }

      //go through right side
      for(int i = startRow;i<endRow;i++){
        //printf("Rank %d Right Side Update %d\n",rank,SIZE*i+SIZE-1);
        *(currentValues + (SIZE * i) + SIZE-1) = G * (*(prevValues + (SIZE * i) + SIZE - 2));
      }

      //do the top
      if(rank == 0){
        for(int j = 1;j<SIZE-1;j++){
          //printf("Rank %d Top: %d\n",rank,j);
          *(currentValues+j) = G * (*(prevValues + SIZE + j));
        }
      }

      //do the bottom
      if(rank==numProcess-1){
        for(int j = 1;j<SIZE-1;j++){
          //printf("Rank %d Bottom: %d    %d\n",rank,SIZE*(endRow)+j,SIZE*(endRow-1) + j);
          *(currentValues + SIZE*(endRow) + j) = G * (*(currentValues + SIZE*(endRow-1) + j));
       }
      }

      //completed sides now corners
      MPI_Barrier(MPI_COMM_WORLD);

      //top corners
      if(rank == 0){
        *(currentValues) = G * (*(currentValues + SIZE));
        *(currentValues+SIZE-1) = G * (*(currentValues + 2 * SIZE -1));
      }
      //bottom corners
      if(rank == numProcess-1){
        //bottom left
        *(currentValues +SIZE*(endRow)) = G * (*(currentValues + SIZE*(endRow) + 1));
        //bottom right
        *(currentValues + SIZE*(endRow) + SIZE -1) = G * (*(currentValues + SIZE*(endRow) + SIZE -2));
      }
    }

    // Finalize the MPI environment.
    free(currentValues);
    free(prevValues);
    free(prevprevValues);
    free(rowBelow);
    free(rowAbove);

    MPI_Finalize();
}
