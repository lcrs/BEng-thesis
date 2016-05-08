/* Reduces the number of points in a slice file by throwing some of them
   away - some kind of interpolation might be wise.
   Use: "slicedecimate n input.sli output.sli" where n is the decimation factor. 
   Make sure to use an integer factor of the points-per-slice count */

#include <stdio.h>

#define MAXLINELENGTH 128

int main(int argc, char *argv[]) {
	FILE	*inputFile, *outputFile;
	char	inputLine[MAXLINELENGTH];
	int	pointCount = 0;
	float	x, y;
	int	pointsPerSlice;
	int	factor;

	if(argc != 4 || strcmp("-h", argv[1]) == 0) {
		printf("Usage: slicedecimate n input.sli output.sli\nWhere n is a factor of the number of points in each slice.\n");
		exit(1);
	}

	factor = atoi(argv[1]);

	inputFile = fopen(argv[2], "r");
	if(inputFile == NULL) {
		printf("Failed to open %s.\n", argv[2]);
		exit(1);
	}
	outputFile = fopen(argv[3], "w");
	if(outputFile == NULL) {
		printf("Failed to open %s.\n", argv[3]);
		exit(1);
	}

	while(fgets(inputLine, MAXLINELENGTH, inputFile)) {
		if(sscanf(inputLine, "%f %f", &x, &y) == 2) {
			/* Read a point data line */
			if(pointCount % factor == 0) {
				/* Keep this point */
				fprintf(outputFile, "%s", inputLine);
			}
			pointCount++;
		} else if(sscanf(inputLine, "points %d", &pointsPerSlice)) {
			/* Write correct points-per-slice header */
			if((float)pointsPerSlice / (float)factor != (float)(pointsPerSlice / factor)) {
				printf("Factor must be a factor of the points-per-slice count!\n");
				exit(1);
			}
			fprintf(outputFile, "points %d\n", pointsPerSlice / factor);
		} else {
			/* Write the line as it was read */
			fprintf(outputFile, "%s", inputLine);
		}
	}

	close(inputFile);
	close(outputFile);
	return(0);
}
