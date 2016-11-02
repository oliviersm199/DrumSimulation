#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TAG 0
#define ETA 0.0002
#define RHO 0.5
#define G 0.75
#define SIZE 4
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

    //printf("WorkEach%d,Size%d\n",workEach,SIZE);

    //row below and row above buffers for interior
    float *rowBelow = (float *)calloc(SIZE,sizeof(float));
    float *rowAbove = (float *)calloc(SIZE,sizeof(float));

    // strike the drum in the center with a value of 1, but only in the process which really has the minimum
    int minIndex = startingRow * SIZE;
    int maxIndex = minIndex + workEach * SIZE -1;
    int indexVal = STRIKEX * SIZE + STRIKEY;

    if( indexVal >= minIndex && indexVal <= maxIndex){
      //printf("Min Value %d, Top Value %d  Index Val %d   Actual Offset in Proc:%d\n",minIndex,maxIndex, indexVal,indexVal -minIndex);
      *(currentValues + indexVal - minIndex) = 1;
    }

    MPI_Barrier(MPI_COMM_WORLD);

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


          int leftIndex = startRow * SIZE + column - 1;
          int rightIndex = startRow * SIZE + column +1;

          int bottomIndex = (startRow+1) * SIZE + column;
          float *bottomPtr = prevValues;
          if(bottomIndex>workEach*SIZE){
            bottomPtr = rowBelow;
            bottomIndex = column;
          }

          int topIndex = (startRow-1) * SIZE + column;
          float*topPtr = prevValues;
          if(topIndex<0){
            topPtr = rowAbove;
            topIndex = column;
          }

          int currentIndex = (startRow) * SIZE + column;

          //printf("Left:(%d,%.6f) Right:(%d,%.6f) Bottom:(%d,%.6f) Top:(%d,%.6f) Prev:(%d,%.6f), PrevPrevValues %.6f\n",leftIndex, *(prevValues+leftIndex),rightIndex,*(prevValues+rightIndex), bottomIndex,*(bottomPtr+bottomIndex),topIndex,*(topPtr+topIndex),currentIndex,*(prevValues+currentIndex),*(prevprevValues+currentIndex));
          float leftValue = *(prevValues+leftIndex);
          float rightValue = *(prevValues+rightIndex);
          float bottomValue = *(bottomPtr+bottomIndex);
          float topValue = *(topPtr+topIndex);
          float prevValue = *(prevValues+currentIndex);
          float prevPrevValue = *(prevprevValues+currentIndex);
          float newValue = ((RHO * (leftValue + rightValue + bottomValue + topValue- 4 * prevValue)) + 2 * prevValue - (1-ETA) * prevPrevValue)/(1 + ETA);
          *(currentValues+currentIndex)=newValue;
          //printf("Iteration: %d Value %.6f, Position: %d Rank %d\n",i,*(currentValues+currentIndex),SIZE*SIZE/numProcess*rank+currentIndex,rank);
      }
        startRow++;
      }


      MPI_Barrier(MPI_COMM_WORLD);

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
        int leftOffset = startRow * SIZE;
        float newLeftVal = G * (*(currentValues +leftOffset+1));
        *(currentValues + leftOffset) =  newLeftVal;
        int actualPosition = SIZE * SIZE / numProcess * rank;
        //printf("from %d to %d   %.6f\n",leftOffset+1+actualPosition,leftOffset+actualPosition,newLeftVal);
      }

      //go through right side
      for(int i = startRow;i<endRow;i++){
        int rightOffset = (SIZE * i) + SIZE - 1;
        float newRightVal = G * (*(currentValues + rightOffset-1));
        *(currentValues + rightOffset) = newRightVal;
        int actualPosition = SIZE * SIZE / numProcess * rank;
        //printf("from %d to %d   %.6f\n",rightOffset-1+actualPosition,rightOffset+actualPosition,newRightVal);
      }

      //do the top
      if(rank == 0){
        for(int j = 1;j<SIZE-1;j++){
          *(currentValues + j) = G * (*(currentValues + SIZE + j));
          //printf("J: %d  Value: %.6f\n",j,G * (*(currentValues + SIZE + j)));
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
          //printf("Bottom %d  BottomTop %d Value %.6f\n",bottomIndex + actualPosition, bottomTop + actualPosition,bottomValue);
       }
      }

      MPI_Barrier(MPI_COMM_WORLD);
      //top corners
      if(rank == 0){
        *(currentValues) = G * (*(currentValues + 1));
        *(currentValues+SIZE-1) = G * (*(currentValues + SIZE - 2));
        //printf("Top Corners: %.6f  %.6f \n",G * (*(currentValues + 1)),G * (*(currentValues + SIZE - 2)));
      }
      //bottom corners
      if(rank == numProcess-1){
        int offset = SIZE * (workEach-1);
        *(currentValues + offset) = G * (*(currentValues + offset + 1));
        //printf("\n%.6f\n",*(currentValues + offset + 1));
        *(currentValues + offset + SIZE -1) = G * (*(currentValues + offset + SIZE - 2));
        //printf("Bottom Corners: %.6f   %.6f\n",G * (*(currentValues + offset + 1)),G * (*(currentValues + offset + SIZE - 2)));
      }

      if( indexVal >= minIndex && indexVal <= maxIndex){
        printf("%.6f\n",*(currentValues + indexVal - minIndex));
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
