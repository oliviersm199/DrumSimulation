// Adapted from code at mpitutorial.com
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


#define ETA 0.0002
#define RHO 0.5
#define G 0.75
#define SIZE 4


int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);


    //get the number of iterations
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


    // each 16 processes will be mapped to a particular point. 
    //    0  1   2    3 
    //    4  5   6    7 
    //    8  9   10   11 
    //    12 13  14   15

    int xPosition = rank % SIZE; // get the x value 
    int yPosition = rank / SIZE; // get the y value

    // set the default starting value of the drum to zero. 
    // otherwise set to 1 if at position SIZE/2
    double value = 0.0;
    if(xPosition==SIZE/2 && yPosition == SIZE/2){
        value = 1.0;
    }


    double prevPrevValue = 0;
    double prevValue = 0;



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
        else if(yPosition==0 && XPosition >=1 && XPosition <= SIZE-2){

        }
        //right side
        else if(yPosition==SIZE-1 && XPosition >=1 && XPosition <= SIZE-2){

        }
        //interior element
        else{

        }
    }


    // Finalize the MPI environment.
    MPI_Finalize();
}
