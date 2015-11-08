#include <mpi.h>
#include <stdio.h>
#include <limits.h>

int main(int argc, char** argv){
	if(argc < 2){
		printf("Not enough arguments. Provide input file location in the rerun\n");
		return -27;
	}
	else{
			// initialize MPI and get rank, size
		double start, end;
		FILE* fout = fopen("max-mpi-ppv1-out.txt", "w");
		fprintf(fout, "Location of the input file is %s\n", argv[1]);
		fclose(fout);
		MPI_Init(NULL, NULL);
		int world_size, world_rank;
		MPI_Comm_size(MPI_COMM_WORLD, &world_size);
		MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
		MPI_Barrier(MPI_COMM_WORLD);
		start = MPI_Wtime();

		// eachSize will hold the value how many ints one processor will process
		int eachSize;

		// basically what happens inside this block is, the root processor reads int values from a file and equally
		// distributes them to other processors
		if(world_rank == 0){
			char *path = argv[1];
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
			        int num;
			        int n;

			        while (sscanf(start, "%d%n", &num, &n) == 1) {
	                	start += n;
			            integers[count] = num;
			            count++;
			            // printf("Added integer %d at index %d\n", num, count);
			        }

			        // puts("");
			    }
			    // printf("error?");
			    fclose(in);

			    int i;

			    // printf("what's happening ere");

			    // THINK HERE WHAT COULD HAPPEN IF NUM OF PROCESSORS IS LAGER THAN NUM OF INTEGERS...

			    if(world_size - 1 > count + 1){
			    	// printf("ord ont'");
			    	// send all the integers to just one worker then
			    	// since we don't have a supercomputer to work this here it means we have a very small data set and 1 CPU can handle it
			    	eachSize = 1;
			    	MPI_Send(&eachSize, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
			    	MPI_Send(integers, count, MPI_INT, 1, 0, MPI_COMM_WORLD);
			    }
			    else{
			    	//send each worker ~equal~ pieces
			    	// printf("send messages\n");

			    	// printf("World size %d\n", world_size);

			    	// don't skip an element, make sure number of elements are divisible by number of processors, if not send the element at the end too
			    	if((count) % (world_size - 1) == 0){
			    		eachSize = (count) / (world_size - 1);
			    	}
			    	else{
			    		eachSize = (count + 1) / (world_size - 1);
			    	}


			    	// printf("Each size: %d\n", eachSize);
			    	for(i = 1; i < world_size; i++){
			    		// (i - 1) * eachSize => offset
			    		// send integers + (i - 1) * eachSize;
			    		// data, count, datatype, destination, tag, communicator
			    		// send array size first, then the part of the array (increment the pointer)
			    		MPI_Send(&eachSize, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
			    		MPI_Send(integers + ((i - 1) * eachSize), eachSize, MPI_INT, i, 0, MPI_COMM_WORLD);		    	
					}
				}

				// finalMax will go through the local max values from the processors
				int finalMax = INT_MIN;
				int curMax;
				for(i = 1; i < world_size; i++){
					// receive local max from each processor and then check if it is larger than the current max, if it is assign it to current max
					MPI_Recv(&curMax, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					if(curMax > finalMax){
						finalMax = curMax;
					}
				}

				FILE* fout = fopen("max-mpi-ppv1-out.txt", "a");
				if(fout != NULL){
					fprintf(fout, "Total number of integers read: %d\nOverall max value is %d\n", count, finalMax);
				}
				else{
					printf("Error opening file.\n");
				}

				printf("Overall max value is %d\n", finalMax);
			}
		}
		else{
			// data, count, type, source, tag, comm, status
			MPI_Recv(&eachSize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			int localInts[eachSize];
			MPI_Recv(localInts, eachSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			int j;
			int localMax = INT_MIN;
			int cur; 
			// go through the received part of the array and find the largest value there
			for(j = 0; j < eachSize; j++){
				cur = localInts[j];
				// printf("At processor %d, index %d value %d\n", world_rank, j, cur);
				if(cur > localMax){
					localMax = cur;
				}
			}

			// send the local maximum value to the root processor
			FILE* out = fopen("max-mpi-ppv1-out.txt", "a");
			if(out != NULL){
				fprintf(out, "Local max value at processor %d is %d\n", world_rank, localMax);
			}
			else{
				printf("Error opening file\n");
			}
			fclose(out);
			printf("Local max value at processor %d is %d\n", world_rank, localMax);
			MPI_Send(&localMax, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		}

		MPI_Barrier(MPI_COMM_WORLD);
		end = MPI_Wtime();

		if(world_rank == 0){
			fprintf(fout, "Runtime: %f\n", end-start);
			printf("Runtime: %f\n", end-start);
			fclose(fout);
		}

		MPI_Finalize();

		return 0;
	}
}