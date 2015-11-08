#include <stdio.h>
#include <limits.h>
#include <mpi.h>
#include <math.h>

int main(int argc, char** argv){
	if(argc < 3){
		printf("Not enough arguments. Provide input file locations in the rerun\n");
		return -27;
	}
	else{
		FILE* fout = fopen("matmult-mpi1-1d.txt", "w");
		fprintf(fout, "");
		fclose(fout);
		MPI_Init(NULL, NULL);
		double start, end;
		int world_rank, world_size;
		MPI_Comm_size(MPI_COMM_WORLD, &world_size);
		MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
		// int dim;
		// put a barrier before measuring the start time to avoid overhead of other cpu's
		MPI_Barrier(MPI_COMM_WORLD);
		start = MPI_Wtime();


		// read from file and dispatching blocks stuff is done in the root processor and as I understand no blocks will be dispatched to the root
		if(world_rank == 0){
			// printf("Enter path to read the integers from: ");
			// scanf("%s", &path);
			char* path1 = argv[1];
			char* path2 = argv[2];
			int **integers1;
			int **integers2;
			int dim;
			//create file pointer
			FILE* in1;
			in1 = fopen(path1, "r");
			if (in1 != NULL) {
			    char line[BUFSIZ];
			    int lineCount = 0;
			    int count = 0; 

			    dim = atoi(fgets(line, sizeof line, in1));
			    // printf("Dimension is %d\n", dim);
			    integers1 = (int**)malloc(dim * sizeof(int*));
			    // read integers line by line first and then split the integers that have whitespace in between 
			    while (fgets(line, sizeof line, in1) != NULL) {
			        char *start = line;
			        int num;
			        int n;
			        // this will be an entire row
			        integers1[lineCount] = (int*)malloc(dim * sizeof (int));
			        count = 0;
			        while (sscanf(start, "%d%n", &num, &n) == 1) {
			        	// add column values to a specific row now
		            	start += n;
			            integers1[lineCount][count] = num;
			            count++;
			            // printf("Added integer %d at row %d and column %d\n", num, lineCount, count);
			        }

			        lineCount++;
			    }

			    fclose(in1);
			}

			//create file pointer
			//make it accesible through column number, because this 2d array will be passed to the other processors in blocks of columns
			FILE* in2;
			in2 = fopen(path2, "r");
			if (in2 != NULL) {
			    char line[BUFSIZ];
			    int lineCount = 0;
			    int count = 0; 

			    dim = atoi(fgets(line, sizeof line, in2));
			    // printf("Dimension is %d\n", dim);
			    integers2 = (int**)malloc(dim * sizeof(int*));
			    int i;
			    // have to allocate space for the entire array if we're gonna add the values column by column
			    for(i = 0; i < dim; i++){
			    	integers2[i] = (int*)malloc(dim * sizeof(int));
			    }
			    // read integers by the way this code will check if there are multiple numbers in lines as well, although the assignment discards the possibility
			    // the code portion shall be usable for part b
			    while (fgets(line, sizeof line, in2) != NULL) {
			        char *start = line;
			        int num;
			        int n;

			        count = 0;
			        // add the second matrice's elements column by column to allow the data to be passed by an int* pointer
			        // otherwise I'd have to pass a double array int** and navigate through columns by ints[k][j], where j is constant and k is
			        // a variable and this is much more memory efficient since c doesn't store 2d arrays as column-major but row-major
			        // but with this trick I'll be storing it the transpose of the second matrix where columns are rows
			        while (sscanf(start, "%d%n", &num, &n) == 1) {
		            	start += n;
			            integers2[count][lineCount] = num;
			            count++;
			            // printf("Added integer %d at column %d and row %d\n", num, lineCount, count);
			        }

			        lineCount++;
			    }

			    fclose(in2);
			}

			// find how many row-column pairs will be distributed to each processing element

			int numRowColPair = (dim*dim)/(world_size-1);
			// int times = (int)sqrt(numRowColPair); // how many rows and how many columns will be passed to a CPU
			int i;
			int total = 0;
			int curCPU = 0;


			for(i = 1; i < world_size; i++){
				// printf("Dim %d is sent to p %d\n", dim, i);
				MPI_Send(&dim, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			}

			// deal each processor row and column vectors
			// the order is start with the rows of the first matrix
			// distribute the column vectors until enough number of row&column pairs is reached
			// if not reached, distribute the next row to the current cpu

			// tags are also created in a unique way for any given vector
			// for a row vector send i + (dim*dim) because max possible value for i in the loop is (dim*dim)
			// for column values just send i as tag
			// so this way, if a tag is greater than (dim*dim) that data is known to be a row vector, otherwise column
			for(i = 0; i < dim*dim; i++){
				// TO DIFFERENTIATE BETWEEN ROWS AND COLUMNS ROWS GOT AN EXTRA DIM*DIM IN THEIR TAG
				if(total % dim == 0 || total % numRowColPair == 0){
					// MPI_Send(integers1[i/dim], dim, MPI_INT, curCPU, i + (dim*dim), MPI_COMM_WORLD);
					// printf("Pass row %d to processor %d with tag %d\n", i/dim, curCPU + 1, i + (dim*dim));
					MPI_Send(integers1[i/dim], dim, MPI_INT, curCPU + 1, i + (dim*dim), MPI_COMM_WORLD);
				}
				// MPI_Send(integers2[i%dim], dim, MPI_INT, curCPU, i, MPI_COMM_WORLD);
				MPI_Send(integers2[i%dim], dim, MPI_INT, curCPU + 1, i, MPI_COMM_WORLD);
				// printf("Pass column %d to processor %d with tag %d\n", i%dim, curCPU + 1, i);
				total++;
				curCPU = total/numRowColPair;
			}

			int resultMatrix[dim][dim];
			int curSum;
			// tag procedure is explained in comments before the corresponding send statement
			for(i = 0; i < dim*dim; i++){
				// MPI_Send(&sum, 1, MPI_INT, 0, dim*rowNum + colNum, MPI_COMM_WORLD);
				// printf("Master waits for row %d col %d from cpu %d tag: %d\n", i/dim, i%dim, (i/numRowColPair) + 1, (i/dim)*dim + i%dim);
				MPI_Recv(&curSum, 1, MPI_INT, (i/numRowColPair) +1, (i/dim)*dim + i%dim, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				resultMatrix[i/dim][i%dim] = curSum;
				// printf("Master receives row %d col %d with val %d from cpu %d tag: %d\n", i/dim, i%dim, curSum, (i/numRowColPair) + 1, (i/dim)*dim + i%dim);
			}


			FILE *f = fopen("matmult-mpi-1d-out.txt", "w+");
			if (f == NULL)
			{
			    printf("Error opening output file!\n");
			    exit(1);
			}

			int j;
			fprintf(f, "Location of the first input file: %s\n", path1);
			fprintf(f, "Location of the second input file: %s\n", path2);
			fprintf(f, "Dimension of the matrices is %d\n", dim);
			fprintf(f, "Resulting matrix is:\n");
			for(i = 0; i < dim; i++){
				for(j = 0; j < dim; j++){
					fprintf(f, "%d ", resultMatrix[i][j]);
				}
				fprintf(f, "\n");
			}

			fclose(f);
			for(i = 0; i < dim; ++i){
				free(integers1[i]);
				free(integers2[i]);
			}
			free(integers1);
			free(integers2);
		}
		else{
			// printf("I'm here\n");
			int i;
			int dim;
			MPI_Recv(&dim, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			int total = 0;
			// number row&column pairs for each processors
			int numRowColPair = (dim*dim)/(world_size - 1);
			int curCPU = 0;
			int rowNum = -1;
			int colNum = -1;
			int curRow[dim];
			int curCol[dim];
			int j;
			int sum;

			
			// printf("dim is %d\n", dim);

			for(i = 0; i < dim*dim; i++){
				// printf("I'm here for\n");
				if(total % dim == 0 || total % numRowColPair == 0){
					// receive row
					if(curCPU + 1 == world_rank){
						MPI_Recv(curRow, dim, MPI_INT, 0, i + (dim*dim), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						// printf("Recv row %d to processor %d with tag %d\n", i/dim, curCPU + 1, i + (dim*dim));
					}
					rowNum = (i/dim);
				}
				if(curCPU + 1 == world_rank){
					MPI_Recv(curCol, dim, MPI_INT, 0, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					// printf("Recv col %d to processor %d with tag %d\n", i%dim, curCPU + 1, i + (dim*dim));
					colNum = i%dim;
				}
				else{
					colNum = -1;
				}
				
				// find one element of the result matrix, with result[rowNum][colNum]
				if(colNum != -1){
					sum = 0;
					for(j = 0; j < dim; j++){
						sum += curRow[j] * curCol[j];
					}
					// so that when the tag is divided by dim it will return the row number, and the remainder will be the colunm number
					MPI_Send(&sum, 1, MPI_INT, 0, dim*rowNum + colNum, MPI_COMM_WORLD);
					// printf("Sent element row %d col %d val %d to root from processor %d tag: %d\n", rowNum, colNum, sum, world_rank, dim*rowNum + colNum);
				}
				
				total++;
				curCPU = total/numRowColPair;
			}
			
		}

		// put a barrier before measuring the end time to make sure all work is finished
		MPI_Barrier(MPI_COMM_WORLD);
		end = MPI_Wtime();


		MPI_Finalize();

		// print runtime (end time in root cpu minus start time in root cpu)
		if (world_rank == 0) { /* use time on master node */
			FILE* f = fopen("matmult-mpi-1d-out.txt", "a");
			if(f != NULL){
				fprintf(f, "Runtime: %f\n", end-start);
			}
		    printf("Runtime: %f\n", end - start);
		    fclose(f);
		}

		return 0;
	}
}