/* Adds or subtracts the vectors in .sli files from one another.
   Use: "slicearithmetic + one.sli two.sli output.sli" adds one.sli to
   two.sli and stores result in output.sli.
   Or, "slicearithmetic - one.sli two.sli output.sli" subtracts one.sli 
   from two.sli and stores result in output.sli. */

#include <stdio.h>

#define MAXLINELENGTH 128

int main(int argc, char *argv[]) {
	FILE	*inputFile1, *inputFile2, *outputFile;
	char	inputLine1[MAXLINELENGTH], inputLine2[MAXLINELENGTH];
	float	x1, y1, x2, y2, xout, yout;

	if(argc != 5 || strcmp("-h", argv[1]) == 0) {
		printf("Usage: slicearithmetic + a.sli b.sli output.sli (to add two slice sets)\n       slicearithmetic - a.sli b.sli output.sli (to subtract a.sli from b.sli)\n");
		exit(1);
	}

	inputFile1 = fopen(argv[2], "r");
	if(inputFile1 == NULL) {
		printf("Failed to open %s.\n", argv[2]);
		exit(1);
	}
	inputFile2 = fopen(argv[3], "r");
	if(inputFile2 == NULL) {
		printf("Failed to open %s.\n", argv[3]);
		exit(1);
	}
	outputFile = fopen(argv[4], "w");
	if(outputFile == NULL) {
		printf("Failed to open %s.\n", argv[4]);
		exit(1);
	}

	while(fgets(inputLine1, MAXLINELENGTH, inputFile1) && fgets(inputLine2, MAXLINELENGTH, inputFile2)) {
		if(sscanf(inputLine1, "%f %f", &x1, &y1) == 2 && sscanf(inputLine2, "%f %f", &x2, &y2) == 2) {
			/* Do the arithmetic and write the point data line */
			if(strcmp(argv[1], "+") == 0) {
				xout = x1 + x2;
				yout = y1 + y2;
			} else {
				xout = x2 - x1;
				yout = y2 - y1;
			}
			fprintf(outputFile, "%f %f\n", xout, yout);
		} else {
			/* Not a point data line, write it as read */
			fprintf(outputFile, "%s", inputLine1);
		}
	}

	close(inputFile1);
	close(inputFile2);
	close(outputFile);
	return(0);
}
