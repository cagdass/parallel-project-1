#include <stdio.h>
#include <limits.h>
#include <time.h>

/* Created by Cagdas Oztekin, Bilkent University, for CS426 Parallel Computing
	Yes it's for a parallel computing course but this program is serial 
	- meant to be a benchmark for the parallel programs
*/

int main(int argc, char** argv){
	
	clock_t begin, end;
	double time_spent;

	if(argc < 3){
		printf("Not enough arguments. Provide input file locations in the rerun\n");
		return -27;
	}
	else{
		begin = clock();
		
		// printf("Enter path to read the integers from: ");
		// scanf("%s", &path);
		char* path1 = argv[1];
		char* path2 = argv[2];
		int dim;
		// the matrices content will be pointed by these guys
		int **integers1;
		int **integers2;

		//create file pointer
		FILE* in1;
		in1 = fopen(path1, "r");
		if (in1 != NULL) {
		    char line[BUFSIZ];
		    int lineCount = 0;
		    int count = 0; 

		    // first line of the matrix is the dimension of the matrix, get that
		    dim = atoi(fgets(line, sizeof line, in1));
		    printf("Dimension is %d\n", dim);
		    integers1 = (int**)malloc(dim * sizeof(int*));
		    // read integers by the way this code will check if there are multiple numbers in lines as well, although the assignment discards the possibility
		    // the code portion shall be usable for part b
		    while (fgets(line, sizeof line, in1) != NULL) {
		        char *start = line;
		        int num;
		        int n;
		        integers1[lineCount] = (int*)malloc(dim * sizeof (int));
		        count = 0;
		        while (sscanf(start, "%d%n", &num, &n) == 1) {
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
		FILE* in2;
		in2 = fopen(path2, "r");
		if (in2 != NULL) {
		    char line[BUFSIZ];
		    int lineCount = 0;
		    int count = 0; 

		    dim = atoi(fgets(line, sizeof line, in2));
		    printf("Dimension is %d\n", dim);
		    integers2 = (int**)malloc(dim * sizeof(int*));
		    // read integers by the way this code will check if there are multiple numbers in lines as well, although the assignment discards the possibility
		    // the code portion shall be usable for part b
		    while (fgets(line, sizeof line, in2) != NULL) {
		        char *start = line;
		        int num;
		        int n;
		        integers2[lineCount] = (int*)malloc(dim * sizeof(int));
		        count = 0;
		        while (sscanf(start, "%d%n", &num, &n) == 1) {
	            	start += n;
		            integers2[lineCount][count] = num;
		            count++;
		            // printf("Added integer %d at row %d and column %d\n", num, lineCount, count);
		        }

		        lineCount++;
		    }

		    fclose(in2);
		}

		// allocate space for the result matrix
		int** resultMatrix = (int**)malloc(dim * sizeof(int*));
		int curSum;
		int i,j,k;

		// runs in O(n^3) time
		// perform a vector multiplication for each row & column pair 
		// (row from the first matrix, column from the second matrix)
		for(i = 0; i < dim; i++){
			resultMatrix[i] = (int*)malloc(dim * sizeof(int));
			for(j = 0; j < dim; j++){
				curSum = 0;
				for(k = 0; k < dim; k++){
					curSum += integers1[i][k] * integers2[k][j];
				}
				resultMatrix[i][j] = curSum;
			}
		}
	
		// print the result
		for(i = 0; i < dim; i++){
			for(j = 0; j < dim; j++){
				// printf("%d ", resultMatrix[i][j]);
			}
			// printf("\n");
		}

		for(i = 0; i < dim; i++){
			free(integers1[i]);
			free(integers2[i]);
			free(resultMatrix[i]);
		}

		free(integers1);
		free(integers2);
		free(resultMatrix);

		FILE* f = fopen("matmult-serial-out.txt", "w");
		if(f != NULL){
			fprintf(f, "Location of the first input file: %s\n", path1);
			fprintf(f, "Location of the second input file: %s\n", path2);
			fprintf(f, "Dimension of the matrices is %d\n", dim);
			fprintf(f, "Resulting matrix is:\n");
			int i, j;
			for(i = 0; i < dim; ++i){
				for(j = 0; j < dim; ++j){
					fprintf(f, "%d ", resultMatrix[i][j]);
				}
				fprintf(f, "\n");
			}
		}

		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		if(f != NULL){
			fprintf(f, "Runtime: %f\n", time_spent);
		}
		fclose(f);
		printf("Runtime: %f\n", time_spent);

		return 0;
	}
	
}