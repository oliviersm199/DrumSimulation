#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define rho 0.5
#define eta 0.0002
#define G 0.75
#define TAG 0

// the struct that contains all the displacement of each node
struct nodes{
    double u;
    double u1;
    double u2;
}nodes;
// the struct that contains all the displacement from the node above him 
struct up
{
    double u;
    double u1;
    double u2; 
}up;
// the struct that contains all the displacement from the node below him
struct down
{
    double u;
    double u1;
    double u2; 
}down;
// the struct that contains all the displacement from the node left to him
struct left
{
    double u;
    double u1;
    double u2; 
}left;
// the struct that contains all the displacement from the node right to him
struct right
{
    double u;
    double u1;
    double u2; 
}right;
// to calculate the displacement of the inner nodes 
double calculation(double up,double down, double left, double right,double processU1,double processU2){
   double temp = (rho*( up+down+left+right - 4 *processU1)+ 2 * processU1 - (1 - eta) * processU2) / (1 + eta);
   return temp; 
}
//to calculate the displacement of the side and corner nodes 
double calculationBC(double neighbour){
    double temp = G*neighbour;
    return temp;
}
int main(int argc, char** argv) {
// save the number of iterations from command line 	
int T = atoi(argv[1]);
MPI_Request request[28];
MPI_Status status;

// the process data
double processU, processU1,newValue;
MPI_Datatype nodeType;
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//create a new data type 
    MPI_Type_contiguous(3,MPI_DOUBLE,&nodeType);
    MPI_Type_commit(&nodeType);
// the first hit at node (N/2, N/2)   
    	if (rank==10){
        	nodes.u=1;
        	nodes.u1=1;
    	}
	
    while(T>0){
    	// rank 5 , 6 , 9 10 are the inner nodes. They will send thier displacement to other inner nodes
    	//before to the side nodes. Once their current vertical displacement is updated, they will send 
    	//thier displacement to the side nodes.
    	if (rank==5){

            MPI_Isend(&nodes,1,nodeType,9,TAG,MPI_COMM_WORLD,&request[2]);
            MPI_Wait(&request[2], &status);

            MPI_Isend(&nodes,1,nodeType,6,TAG,MPI_COMM_WORLD,&request[1]);
            MPI_Wait(&request[1], &status); 

            ////////////////////////////////////////////////////////
            MPI_Irecv(&up,1,nodeType,1,TAG,MPI_COMM_WORLD,&request[3]);
            MPI_Wait(&request[3], &status); 

            MPI_Irecv(&left,1,nodeType,4,TAG,MPI_COMM_WORLD,&request[4]);
            MPI_Wait(&request[4], &status); 

            MPI_Irecv(&down,1,nodeType,9,TAG,MPI_COMM_WORLD,&request[5]);
            MPI_Wait(&request[5], &status); 

            MPI_Irecv(&right,1,nodeType,6,TAG,MPI_COMM_WORLD,&request[6]);
            MPI_Wait(&request[6], &status);

            newValue=calculation(up.u1,down.u1,left.u1,right.u1,nodes.u1,nodes.u2);

            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue; 
            MPI_Isend(&nodes,1,nodeType,1,TAG,MPI_COMM_WORLD,&request[7]);
            MPI_Wait(&request[7], &status);

            MPI_Isend(&nodes,1,nodeType,4,TAG,MPI_COMM_WORLD,&request[8]);
            MPI_Wait(&request[8], &status);

        }
        if (rank==6){
            

            MPI_Isend(&nodes,1,nodeType,10,TAG,MPI_COMM_WORLD,&request[9]);
            MPI_Wait(&request[9], &status);


            MPI_Isend(&nodes,1,nodeType,5,TAG,MPI_COMM_WORLD,&request[6]);
            MPI_Wait(&request[6], &status);

            ////////////////////////////////////////////////////////
            MPI_Irecv(&up,1,nodeType,2,TAG,MPI_COMM_WORLD,&request[10]);
            MPI_Wait(&request[10], &status);


            MPI_Irecv(&right,1,nodeType,7,TAG,MPI_COMM_WORLD,&request[11]);
            MPI_Wait(&request[11], &status);

            MPI_Irecv(&down,1,nodeType,10,TAG,MPI_COMM_WORLD,&request[12]);
            MPI_Wait(&request[12], &status);

            MPI_Irecv(&left,1,nodeType,5,TAG,MPI_COMM_WORLD,&request[1]);
            MPI_Wait(&request[1], &status); 

            newValue=calculation(up.u1,down.u1,left.u1,right.u1,nodes.u1,nodes.u2);

            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;

            MPI_Isend(&nodes,1,nodeType,2,TAG,MPI_COMM_WORLD,&request[13]);
            MPI_Wait(&request[13], &status);

            MPI_Isend(&nodes,1,nodeType,7,TAG,MPI_COMM_WORLD,&request[14]);
            MPI_Wait(&request[14], &status);

            }
        if (rank==9){
            MPI_Isend(&nodes,1,nodeType,5,TAG,MPI_COMM_WORLD,&request[5]);
            MPI_Wait(&request[5], &status); 

            MPI_Isend(&nodes,1,nodeType,10,TAG,MPI_COMM_WORLD,&request[15]);
            MPI_Wait(&request[15], &status);

           
            ////////////////////////////////////////////////////////
            MPI_Irecv(&up,1,nodeType,5,TAG,MPI_COMM_WORLD,&request[2]);
            MPI_Wait(&request[2], &status);


            MPI_Irecv(&right,1,nodeType,10,TAG,MPI_COMM_WORLD,&request[16]);
            MPI_Wait(&request[16], &status);

            MPI_Irecv(&down,1,nodeType,13,TAG,MPI_COMM_WORLD,&request[17]);
            MPI_Wait(&request[17], &status);

            MPI_Irecv(&left,1,nodeType,8,TAG,MPI_COMM_WORLD,&request[18]);
            MPI_Wait(&request[18], &status);

            newValue=calculation(up.u1,down.u1,left.u1,right.u1,nodes.u1,nodes.u2);

            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;

            MPI_Isend(&nodes,1,nodeType,13,TAG,MPI_COMM_WORLD,&request[19]);
            MPI_Wait(&request[19], &status);

            MPI_Isend(&nodes,1,nodeType,8,TAG,MPI_COMM_WORLD,&request[20]);
            MPI_Wait(&request[20], &status); 
            }

        if (rank==10){
            MPI_Isend(&nodes,1,nodeType,6,TAG,MPI_COMM_WORLD,&request[12]);
            MPI_Wait(&request[12], &status);


            MPI_Isend(&nodes,1,nodeType,9,TAG,MPI_COMM_WORLD,&request[16]);
            MPI_Wait(&request[16], &status); 
            ////////////////////////////////////////////////////////
            MPI_Irecv(&up,1,nodeType,6,TAG,MPI_COMM_WORLD,&request[9]);
            MPI_Wait(&request[9], &status);


            MPI_Irecv(&right,1,nodeType,11,TAG,MPI_COMM_WORLD,&request[21]);
            MPI_Wait(&request[21], &status); 


            MPI_Irecv(&down,1,nodeType,14,TAG,MPI_COMM_WORLD,&request[22]);
            MPI_Wait(&request[22], &status); 

            MPI_Irecv(&left,1,nodeType,9,TAG,MPI_COMM_WORLD,&request[15]);
            MPI_Wait(&request[15], &status);

            newValue=calculation(up.u1,down.u1,left.u1,right.u1,nodes.u1,nodes.u2);

            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;


            MPI_Isend(&nodes,1,nodeType,11,TAG,MPI_COMM_WORLD,&request[23]);
            MPI_Wait(&request[23], &status); 

            MPI_Isend(&nodes,1,nodeType,14,TAG,MPI_COMM_WORLD,&request[24]);
             MPI_Wait(&request[24], &status); 
                

            }
        // rank 1,2,7,11,14,13,8,4 are the side nodes. They will send thier displacement to the inner nodes
    	//before to the corner nodes. Once their current vertical displacement is updated, they will send 
    	//thier displacement to the corner nodes.
        if (rank==1){
        	MPI_Isend(&nodes,1,nodeType,5,TAG,MPI_COMM_WORLD,&request[3]);
        	MPI_Wait(&request[3], &status);
        	MPI_Irecv(&down,1,nodeType,5,TAG,MPI_COMM_WORLD,&request[7]);
        	MPI_Wait(&request[7], &status); 
        	newValue = calculationBC(down.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
            MPI_Isend(&nodes,1,nodeType,0,TAG,MPI_COMM_WORLD,&request[25]);
            MPI_Wait(&request[25], &status);
           

        }
        if (rank==4){
        	MPI_Isend(&nodes,1,nodeType,5,TAG,MPI_COMM_WORLD,&request[4]);
        	MPI_Wait(&request[4], &status); 
			MPI_Irecv(&left,1,nodeType,5,TAG,MPI_COMM_WORLD,&request[8]);
        	MPI_Wait(&request[8], &status);
        	newValue = calculationBC(left.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
            
        }
        if (rank==2){
        	MPI_Isend(&nodes,1,nodeType,6,TAG,MPI_COMM_WORLD,&request[10]);
        	MPI_Wait(&request[10], &status);
        	MPI_Irecv(&down,1,nodeType,6,TAG,MPI_COMM_WORLD,&request[13]);
        	MPI_Wait(&request[13], &status);
        	newValue = calculationBC(down.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
            MPI_Isend(&nodes,1,nodeType,3,TAG,MPI_COMM_WORLD,&request[26]);
            MPI_Wait(&request[26], &status);
            
        	
        }
        if (rank==7){
        	MPI_Isend(&nodes,1,nodeType,6,TAG,MPI_COMM_WORLD,&request[11]);
        	MPI_Wait(&request[11], &status);
        	MPI_Irecv(&left,1,nodeType,6,TAG,MPI_COMM_WORLD,&request[14]);
        	MPI_Wait(&request[14], &status);
        	newValue = calculationBC(left.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
         
        }
        if(rank==13){
        	MPI_Isend(&nodes,1,nodeType,9,TAG,MPI_COMM_WORLD,&request[17]);
        	MPI_Wait(&request[17], &status);
			MPI_Irecv(&down,1,nodeType,9,TAG,MPI_COMM_WORLD,&request[19]);
        	MPI_Wait(&request[19], &status);
        	newValue = calculationBC(down.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
          

        }
        if(rank==8){
        	MPI_Isend(&nodes,1,nodeType,9,TAG,MPI_COMM_WORLD,&request[18]);
        	MPI_Wait(&request[18], &status);
        	MPI_Irecv(&left,1,nodeType,9,TAG,MPI_COMM_WORLD,&request[20]);
        	MPI_Wait(&request[20], &status);
        	newValue = calculationBC(left.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
            MPI_Isend(&nodes,1,nodeType,12,TAG,MPI_COMM_WORLD,&request[28]);
        	MPI_Wait(&request[28], &status);
        	
        }
        if(rank==11){
        	MPI_Isend(&nodes,1,nodeType,10,TAG,MPI_COMM_WORLD,&request[21]);
        	MPI_Wait(&request[21], &status);


       
        	MPI_Irecv(&left,1,nodeType,10,TAG,MPI_COMM_WORLD,&request[23]);
        	MPI_Wait(&request[23], &status);

        	newValue = calculationBC(left.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
            MPI_Isend(&nodes,1,nodeType,15,TAG,MPI_COMM_WORLD,&request[27]);
            MPI_Wait(&request[27], &status);
            
        }
        if(rank==14){
        	MPI_Isend(&nodes,1,nodeType,10,TAG,MPI_COMM_WORLD,&request[22]);
        	MPI_Wait(&request[22], &status);
        	
        	MPI_Irecv(&up,1,nodeType,10,TAG,MPI_COMM_WORLD,&request[24]);
        	MPI_Wait(&request[24], &status);

        	newValue = calculationBC(up.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
            
        }
         // rank 0,3,12,15 are the corner nodes.They only receive displacement. 
        if (rank==0){
        	MPI_Irecv(&up,1,nodeType,1,TAG,MPI_COMM_WORLD,&request[25]);
        	MPI_Wait(&request[25], &status);
        	newValue = calculationBC(up.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
        	
        }
        if (rank==3){
        	MPI_Irecv(&up,1,nodeType,2,TAG,MPI_COMM_WORLD,&request[26]);
        	MPI_Wait(&request[26], &status);
        	newValue = calculationBC(up.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
        	

        }
        if(rank==15){
        	MPI_Irecv(&up,1,nodeType,11,TAG,MPI_COMM_WORLD,&request[27]);
        	MPI_Wait(&request[27], &status);
        	newValue = calculationBC(up.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
        	
        }
        if(rank==12){
        	MPI_Irecv(&up,1,nodeType,8,TAG,MPI_COMM_WORLD,&request[28]);
        	MPI_Wait(&request[28], &status);
        	newValue = calculationBC(up.u);
            nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=newValue;
        	
        }
        // make sure all the process are done with sending and receiving 
        MPI_Barrier(MPI_COMM_WORLD);
        // update the nodes of all the process
        if(rank==0||rank==1||rank==2||rank==3||rank==4||rank==5||rank==6||rank==7||rank==8||rank==9||rank==10||rank==11||rank==12||rank==13||rank==14||rank==15){
        	nodes.u2=nodes.u1;
            nodes.u1=nodes.u;
            nodes.u=nodes.u;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank==10){
    	printf("%f,\n",nodes.u);
    }
        T--;

    }
    MPI_Finalize();
}
