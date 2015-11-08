#include <stdio.h>
#include <limits.h>
#include <time.h>

int main(int argc, char** argv){
	clock_t begin, end;
	double time_spent;
	begin = clock();
	if(argc < 2){
		printf("Not enough arguments. Provide input file location in the rerun\n");
		return -27;
	}
	else{
		int integers[10000];
		// printf("Enter path to read the integers from: ");
		// scanf("%s", &path);
		char* path = argv[1];

		//create file pointer
		FILE* in;
		in = fopen(path, "r");
		if (in != NULL) {
		    char line[BUFSIZ];
		    int count = 0; 

		    // read integers by the way this code will check if there are multiple numbers in lines as well, although the assignment discards the possibility
		    // the code portion shall be usable for part b
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

		    }

		    fclose(in);

		    int i;
			int finalMax = INT_MIN;
			int curMax;

			// find the maximum value
			for(i = 0; i < count; i++){
				curMax = integers[i];
				if(curMax > finalMax){
					finalMax = curMax;
				}
			}

			// printf("Overall max value is %d\n", finalMax);

			FILE* out;
			out = fopen("serial-max-integer-out.txt", "w");
			if(out != NULL){
				fprintf(out, "Location of the input file: %s\n", path);
				fprintf(out, "Among %d integers that've been read, the maximum is %d\n", count, finalMax);
				end = clock();
				fprintf(out, "Runtime: %f", (double)(end - begin) / CLOCKS_PER_SEC);
				printf("Done. Runtime: %f\n", (double)(end - begin) / CLOCKS_PER_SEC);
			}

			fclose(out);

			
		}
		
		return 0;
	}
}