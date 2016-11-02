#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TAG 0
#define ETA 0.0002
#define RHO 0.5
#define G 0.75
#define SIZE 512
#define STRIKEX SIZE/2
#define STRIKEY SIZE/2
#define STRIKE 1

void exchange(int sendingProcess, int receiveProcess, float* sendingData,float* receiveBuffer,int size){
  // Request for non-blocking communication
  MPI_Request request;
  MPI_Status status;
  MPI_Sendrecv(sendingData,size,MPI_FLOAT,receiveProcess,TAG,receiveBuffer,size,MPI_FLOAT,receiveProcess,TAG,MPI_COMM_WORLD,&status);
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
    if(iterations < 0){
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

    //once we know how much each is responsible for, we use the rank
    //to determine the row for each process
    int startingRow = workEach * rank;

    //initializing the buffers in each process for their section
    float *currentValues = (float *)calloc(workEach * SIZE,sizeof(float));
    float *prevValues    = (float * )calloc(workEach * SIZE,sizeof(float));
    float *prevprevValues= (float *)calloc(workEach * SIZE,sizeof(float));


    //row below and row above buffers for interior
    float *rowBelow = (float *)calloc(SIZE,sizeof(float));
    float *rowAbove = (float *)calloc(SIZE,sizeof(float));

    // strike the drum in the center with a value of 1, but only in the process which really has the minimum
    int minIndex = startingRow * SIZE;
    int maxIndex = minIndex + workEach * SIZE -1;
    int indexVal = STRIKEX * SIZE + STRIKEY;


    //strike the center value 
    if( indexVal >= minIndex && indexVal <= maxIndex){
      *(currentValues + indexVal - minIndex) = STRIKE;
    }


    for(int i =0;i<iterations;i++){
      //copy the previous values into the prevprev values and the current values into the present values
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

          // calculate the indicies of where we want to update
	  int leftIndex = startRow * SIZE + column - 1;
          int rightIndex = startRow * SIZE + column +1;

          int bottomIndex = (startRow+1) * SIZE + column;
          float *bottomPtr = prevValues;


	  // for rows below and above we need to use buffers if outside of range
          int usingRowBelow = 0;
          if(bottomIndex>workEach*SIZE){
            bottomPtr = rowBelow;
            bottomIndex = column;
            usingRowBelow = 1;
          }

          int topIndex = (startRow-1) * SIZE + column;
          float*topPtr = prevValues;

          int usingRowAbove = 0;
          if(topIndex<0){
            topPtr = rowAbove;
            topIndex = column;
            usingRowAbove = 1;
          }

          int currentIndex = (startRow) * SIZE + column;

	  // get values for interior equation and then calculate the new value
          float leftValue = *(prevValues+leftIndex);
          float rightValue = *(prevValues+rightIndex);
          float bottomValue = *(bottomPtr+bottomIndex);
          float topValue = *(topPtr+topIndex);
          float prevValue = *(prevValues+currentIndex);
          float prevPrevValue = *(prevprevValues+currentIndex);
          float newValue = ((RHO * (leftValue + rightValue + bottomValue + topValue- 4 * prevValue)) + 2 * prevValue - (1-ETA) * prevPrevValue)/(1 + ETA);
          
	  // updating value 
	  *(currentValues+currentIndex)=newValue;
      }
        startRow++;
      }

      //reset the start row and endRow depending on which process you are in 
      startRow=0;
      if(rank == 0){
        startRow +=1;
      }

      endRow = workEach;
      if(rank == numProcess-1){
        endRow -=1;
      }

      //go through left side
      for(int row = startRow; row < endRow;row++){
        int leftOffset = row * SIZE;
        float newLeftVal = G * (*(currentValues +leftOffset+1));
        *(currentValues + leftOffset) =  newLeftVal;
        int actualPosition = SIZE * SIZE / numProcess * rank;
      }

      //go through right side
      for(int row = startRow;row<endRow;row++){
        int rightOffset = (SIZE * row) + SIZE - 1;
        float newRightVal = G * (*(currentValues + rightOffset-1));
        *(currentValues + rightOffset) = newRightVal;
        int actualPosition = SIZE * SIZE / numProcess * rank;
      }

      //do the top
      if(rank == 0){
        for(int j = 1;j<SIZE-1;j++){
          *(currentValues + j) = G * (*(currentValues + SIZE + j));
        }
      }

      //do the bottom
      if(rank==numProcess-1){
        for(int j = 1;j<SIZE-1;j++){
          int offsetStart = endRow*SIZE;
          int bottomIndex = offsetStart + j;
          int bottomTop = bottomIndex - SIZE;
          float bottomValue = G* (*(currentValues + bottomTop));
          *(currentValues + bottomIndex) = bottomValue;
          int actualPosition = SIZE * SIZE / numProcess * rank;
       }
      }


      //top corners
      if(rank == 0){
        *(currentValues) = G * (*(currentValues + 1));
        *(currentValues+SIZE-1) = G * (*(currentValues + SIZE - 2));
      }
      //bottom corners
      if(rank == numProcess-1){
        int offset = SIZE * (workEach-1);
        *(currentValues + offset) = G * (*(currentValues + offset + 1));
        *(currentValues + offset + SIZE -1) = G * (*(currentValues + offset + SIZE - 2));
      }

      //if you are the process that contains the middle value, print it
      if( indexVal >= minIndex && indexVal <= maxIndex){
        printf("%.6f,\n",*(currentValues + indexVal - minIndex));
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
