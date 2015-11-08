#include <mpi.h>
#include <stdio.h>
#include <limits.h>

int main(int argc, char** argv){
	if(argc < 2){
		printf("Not enough arguments. Provide input file location in the rerun\n");
		return -27;
	}
	else{
		FILE* fout = fopen("max-mpi-ppv2-out.txt", "w");
		fprintf(fout, "Location of the input file is %s\n", argv[1]);
		fclose(fout);
		MPI_Init(NULL, NULL);
		int world_size, world_rank;
		double start, end;
		int overallMax = -1;
		int localMax = -INT_MIN;
		MPI_Comm_size(MPI_COMM_WORLD, &world_size);
		MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
		MPI_Barrier(MPI_COMM_WORLD);
		start = MPI_Wtime();
		//get file name
		
		// if(root) read from file all the ints

		int eachSize;

		if(world_rank == 0){
			char* path = argv[1];
			int integers[10000];
			// printf("Enter path to read the integers from: ");
			// scanf("%s", &path);

			//create file pointer
			FILE* in;
			in = fopen(path, "r");
			if (in != NULL) {
			    char line[BUFSIZ];
			    int count = 0; 

			    while (fgets(line, sizeof line, in) != NULL) {
			        char *start = line;
			        int field;
			        int n;

			        while (sscanf(start, "%d%n", &field, &n) == 1) {
			        	// printf("%d \n", field);
	                	start += n;
			            integers[count] = field;
			            count++;
			            // printf("Added integer %d at index %d\n", field, count);
			        }

			        // puts("");
			        // printf("waiting here\n");
			    }
			    // printf("error?");
			    fclose(in);

			    int i;

			    // printf("what's happening ere");

			    // THINK HERE WHAT COULD HAPPEN IF NUM OF PROCESSORS IS LAGER THAN NUM OF INTEGERS...

			    if(world_size - 1 > count + 1){
			    	// just send to one worker then
			    	//send each worker ~equal~ pieces
			    	// printf("send messages\n");
			    	// eachSize = (count + 1) / (world_size - 1);
			    	eachSize = count;
			    	// for(i = 1; i < world_size; i++){

			    		// (i - 1) * eachSize => offset
			    		// send integers + (i - 1) * eachSize;
			    		// data, count, datatype, destination, tag, communicator
			    		// send array size first
			    		MPI_Send(&eachSize, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
			    		MPI_Send(integers, eachSize, MPI_INT, 1, 0, MPI_COMM_WORLD);
								    	
					// }
			    }
			    else{
			    	//send each worker ~equal~ pieces
			    	// printf("send messages\n");
			    	if((count) % (world_size - 1) == 0){
			    		eachSize = (count) / (world_size - 1);
			    	}
			    	else{
			    		eachSize = (count + 1) / (world_size - 1);
			    	}
			    	for(i = 1; i < world_size; i++){

			    		// (i - 1) * eachSize => offset
			    		// send integers + (i - 1) * eachSize;
			    		// data, count, datatype, destination, tag, communicator
			    		// send array size first
			    		MPI_Send(&eachSize, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
			    		MPI_Send(integers + (i - 1) * eachSize,	eachSize, MPI_INT, i, 0, MPI_COMM_WORLD);
								    	
					}
				}

				// int finalMax = INT_MIN;
				// int curMax;
				// for(i = 1; i < world_size; i++){
				// 	MPI_Recv(&curMax, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				// 	if(curMax > finalMax){
				// 		finalMax = curMax;
				// 	}
				// }

				// printf("Overall max value is %d\n", finalMax);
			}
		}
		else{
			// data, count, type, source, tag, comm, status
			MPI_Recv(&eachSize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			int localInts[eachSize];
			MPI_Recv(localInts, eachSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			int j;
			localMax = INT_MIN;
			int cur; 
			for(j = 0; j < eachSize; j++){
				cur = localInts[j];
				// printf("At processor %d, index %d value %d\n", world_rank, j, cur);
				if(cur > localMax){
					localMax = cur;
				}
			}

			FILE* out = fopen("max-mpi-ppv2-out.txt", "a");
			if(out != NULL){
				fprintf(out, "Local max value at processor %d is %d\n", world_rank, localMax);
			}
			else{
				printf("Error opening file.\n");
			}
			fclose(out);

			printf("Local max value at processor %d is %d\n", world_rank, localMax);
		}

		// everything up to here is almost the same but this time send the results to each processor in an all-to-all manner and print the result at each one
		MPI_Allreduce(&localMax, &overallMax, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

		// turns out MPI_Barrier is unnecessary after allreduce!!!
		// MPI_Barrier(MPI_COMM_WORLD);
		
		FILE* out = fopen("max-mpi-ppv2-out.txt", "a");
		if(out != NULL){
			fprintf(out, "Overall value at processor %d is %d\n", world_rank, overallMax);
		}
		else{
			printf("Error opening file");
		}
		if(world_rank != 0){
			fclose(out);
		}
		
		printf("Overall value at processor %d is %d\n", world_rank, overallMax);
		

		MPI_Barrier(MPI_COMM_WORLD);
		end = MPI_Wtime();

		if(world_rank == 0){
			fprintf(out, "Runtime: %f\n", end - start);
			printf("Runtime: %f\n", end - start);
			fclose(out);
		}

		MPI_Finalize();

		return 0;
	}
}