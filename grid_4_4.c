#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


#define ETA 0.0002
#define RHO 0.5
#define G 0.75
#define SIZE 4
#define TAG1 1
#define TAG2 2
#define TAG3 3
#define TAG4 4


int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);


    //get the number of iterations
    int *grid;
    char *ptr;
    long iterations;
    iterations = strtol(argv[1], &ptr, 10);

    if(iterations <=0){
        printf("Invalid input, please input a positive integer for the number of iterations\n");
        exit(-1);
    }

    // Get the number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //if (rank==0){
     grid = malloc(16 *  sizeof(int));
     for (int i=0;i<16;i++){
     	grid[i]=0;
     }
    //}
    // each 16 processes will be mapped to a particular point.
    //    0  1   2    3
    //    4  5   6    7
    //    8  9   10   11
    //    12 13  14   15

    int xPosition = rank % SIZE; // get the x value
    int yPosition = rank / SIZE; // get the y value

    MPI_Request request;
    MPI_Status status;


    // set the default starting value of the drum to zero.
    // otherwise set to 1 if at position SIZE/2
    double value = 0.0;
    if(xPosition==SIZE/2 && yPosition == SIZE/2){
        value = 1.0;
    }


    double prevPrevValue = 0;
    double prevValue = 0;
    double neighbourUp=0;
    double neighbourDown=0;
    double neighbourLeft=0;
    double neighbourRight=0;


    //starting the iterations of the drum
    for(long i =0;i<iterations;i++){
        //send your data
        //wait to receive your data.
        //when you have received your data,



        // top left corner
        if(xPosition == 0 && yPosition==0){


        }
        // top right corner
        else if(xPosition==SIZE-1 && yPosition==0){

        }
        // bottom left corner
        else if(xPosition == 0 && yPosition==SIZE-1){

        }
        //bottom right corner
        else if(xPosition == SIZE-1 && yPosition==SIZE-1){

        }
        //top side
        else if(xPosition == 0 && yPosition >=1 && yPosition <= SIZE-2){

        }
        //bottom side
        else if(xPosition==SIZE-1 && yPosition >=1 && yPosition <= SIZE-2){

        }
        //left side
        else if(yPosition==0 && xPosition >=1 && xPosition <= SIZE-2){

        }
        //right side
        else if(yPosition==SIZE-1 && xPosition >=1 && xPosition <= SIZE-2){

        }
        //interior element
        else{

        	MPI_Isend(&prevValue, 1, MPI_DOUBLE, (4*(yPosition-1))+xPosition, TAG1, MPI_COMM_WORLD, &request);
        	MPI_Isend(&prevValue, 1, MPI_DOUBLE, (4*(yPosition+1))+xPosition , TAG2, MPI_COMM_WORLD, &request);
        	MPI_Isend(&prevValue, 1, MPI_DOUBLE, (4*yPosition)+(xPosition-1), TAG3, MPI_COMM_WORLD, &request);
        	MPI_Isend(&prevValue, 1, MPI_DOUBLE, (4*yPosition-1)+(xPosition+1), TAG4, MPI_COMM_WORLD, &request);
    		MPI_Wait(&request, MPI_STATUS_IGNORE);
    		MPI_Irecv(&neighbourUp,1,MPI_DOUBLE,(4*(yPosition-1))+xPosition,TAG1,MPI_COMM_WORLD,&request);
    		MPI_Irecv(&neighbourDown,1,MPI_DOUBLE,(4*(yPosition+1))+xPosition, TAG2,MPI_COMM_WORLD,&request);
    		MPI_Irecv(&neighbourLeft,1,MPI_DOUBLE,(4*yPosition)+(xPosition-1), TAG3,MPI_COMM_WORLD,&request);
    		MPI_Irecv(&neighbourRight,1,MPI_DOUBLE,(4*yPosition-1)+(xPosition+1), TAG4,MPI_COMM_WORLD,&request);
        }
    }

    if (rank==15){
    int x =0;
	    for (int i =0; i<4;i++){
	    	for (int j =0; j <4;j++){
	    	printf("%d--(%d,%d) ",grid[x],i,j);
	    	x++;
	    	//printf("%d",rank);
	    	}
	    	printf("\n");
	    }
	}


    // Finalize the MPI environment.
    MPI_Finalize();
}
