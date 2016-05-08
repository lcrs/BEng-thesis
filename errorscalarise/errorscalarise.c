/* Takes a template mesh and a deformed mesh and outputs the scalar error
   as a .sli3 of the template mesh with the error as a per-vertex value */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#define MAXLINELENGTH 128

typedef struct vector_t {
	float	x;
	float	y;
} vector_t;

/* Returned by intersect() */
typedef enum intersect_t {
	PARALLEL,
	COINCIDENT,
	SEGMENT_A,
	SEGMENT_B,
	SEGMENT_BOTH,
	SEGMENT_NEITHER
} intersect_t;

/* Global arrays of points */
vector_t	*template, *error;
float		*output;

/* Other global variables */
int		sliceCount, pointsPerSlice;
int		currentPoint = 0;
char		*outputPath;

/* Flag indicating whether we are stepping through and drawing or processing as fast as possible */
int		fastMode = 0;

/* Radius of area to draw around current point, for zooming */
int		scope = 10.0;

/* Return length of a vector */
float length(vector_t *v) {
	return(sqrt((v->x * v->x) + (v->y * v->y)));
}

/* Normalise a vector */
void normalise(vector_t *v) {
	float	l;

	l = length(v);
	v->x /= l;
	v->y /= l;
}

/* Rotate a vector anti-clockwise about the origin */
void rotate(vector_t *v, float radians) {
	float		x, y;

	x = v->x;
	y = v->y;

	v->x = x * cos(radians) - y * sin(radians);
	v->y = x * sin(radians) + y * cos(radians);
}

/* Find the intersection point of two lines given two points on each */
intersect_t intersect(vector_t *a1, vector_t *a2, vector_t *b1, vector_t *b2, vector_t *intersection) {
	float		ua, ub, d;

	d = ((b2->y - b1->y) * (a2->x - a1->x) - (b2->x - b1->x) * (a2->y - a1->y));

	if(d == 0.0) {
		return(PARALLEL);
	}

	ua = ((b2->x - b1->x) * (a1->y - b1->y) - (b2->y - b1->y) * (a1->x - b1->x));
	ub = ((a2->x - a1->x) * (a1->y - b1->y) - (a2->y - a1->y) * (a1->x - b1->x));
	
	if(ua == 0.0 && ub == 0.0) {
		return(COINCIDENT);
	}
	
	ua /= d;
	ub /= d;

	intersection->x = a1->x + ua * (a2->x - a1->x);
	intersection->y = a1->y + ua * (a2->y - a1->y);

	if(ua >= 0.0 && ua <= 1.0) {
		/* Intersection point is within line segment a */
		if(ub >= 0.0 && ub <= 1.0) {
			/* Intersection point is also within line segment b */
			return(SEGMENT_BOTH);
		}
		return(SEGMENT_A);
	} else if(ub >= 0.0 && ub <= 1.0) {
		/* Intersection point is within segment B */
		return(SEGMENT_B);
	} else {
		/* Intersection point is within neither segment */
		return(SEGMENT_NEITHER);
	}
}

/* Write out data from output array to file */
void writeOutput(void) {
	int		i, j;
	FILE		*outputFile;

	/* Write header to new file */
	outputFile = fopen(outputPath, "w");
	if(outputFile == NULL) {
		printf("Failed to open %s\n", outputPath);
		exit(1);
	}

	fprintf(outputFile, "slice3_set\nslices %d\npoints %d\n", sliceCount, pointsPerSlice);

	/* Write data points */
	for(i = 0; i < sliceCount; i++) {
		/* Start of slice header */
		fprintf(outputFile, "slice %d\n", i);
		for(j = 0; j < pointsPerSlice; j++) {
			fprintf(outputFile, "%3.10f %3.10f %3.10f\n", template[i * pointsPerSlice + j].x, template[i * pointsPerSlice + j].y, output[i * pointsPerSlice + j]);
		}
	}
}

/* Conform error function such that each corresponding points in the template and error are nearest neighbours, returning number of changes */
int conform(void) {
	int		i, j, k, l;
	int		indexOfLeast;
	float		least, factor;
	vector_t	delta;
	int		changes = 0;

	/* For each slice in template: */
	for(i = 0; i < sliceCount; i++) {
		printf("Conforming slice %d\r", i);
		fflush(stdout);

		/* For each point on the slice: */
		for(j = 0; j < pointsPerSlice;  j++) {
			/* For each point in slice of error mesh: */
			for(k = 0; k < pointsPerSlice; k++) {
				/* Compute distance to template mesh, accumulating index of least */
				delta.x = (template[i * pointsPerSlice + k].x + error[i * pointsPerSlice + k].x) - template[i * pointsPerSlice + j].x;
				delta.y = (template[i * pointsPerSlice + k].y + error[i * pointsPerSlice + k].y) - template[i * pointsPerSlice + j].y;
				if(k == 0) {
					least = length(&delta);
					indexOfLeast = k;
				} else {
					if(length(&delta) < least) {
						least = length(&delta);
						indexOfLeast = k;
					}
				}
			}
			/* If index of least is not index of current point in template slice */
			if(indexOfLeast != j) {
				/* Scale all errors down by (smallest distance / aligned point-point distance) */
				factor = 0.9 * least / length(&error[i * pointsPerSlice + j]);
				printf("Deformed %d too close to template %d, scaling all error data by %f...\n", indexOfLeast, j, factor);
				for(l = 0; l < sliceCount * pointsPerSlice; l++) {
					error[l].x *= factor;
					error[l].y *= factor;
				}
				changes++;
			}
		}	
	}

	printf("\n");

	return(changes);
}

void scalarise(void) {
	int		r;
	float		x, y;
	vector_t	prev, current, next, normal, tangent, intersection, currentPlusNormal;
	vector_t	prevEdge, nextEdge;
	vector_t	errorPrev, errorCurrent, errorNext;
	vector_t	deformedPrev, deformedCurrent, deformedNext;
	vector_t	vectorError;
	float		scalarError;

	printf("slice %d, point %d\r", currentPoint / pointsPerSlice, currentPoint % pointsPerSlice);
	fflush(stdout);

	current = template[currentPoint];
	errorCurrent = error[currentPoint];

	/* Get vectors for previous and next edges relative to current point */
	if(currentPoint % pointsPerSlice == 0) {
		/* First point in a slice - previous vertex is last in slice */
		prev = template[currentPoint + pointsPerSlice - 1];
		errorPrev = error[currentPoint + pointsPerSlice - 1];
	} else {
		prev = template[currentPoint - 1];
		errorPrev = error[currentPoint  - 1];
	}
	if((currentPoint + 1) % pointsPerSlice == 0) {
		/* Last point in a slice - next vertex is first in slice */
		next = template[currentPoint - pointsPerSlice + 1];
		errorNext = error[currentPoint - pointsPerSlice + 1];
	} else {
		next = template[currentPoint + 1];
		errorNext = error[currentPoint + 1];
	}
	prevEdge.x = current.x - prev.x;
	prevEdge.y = current.y - prev.y;
	nextEdge.x = next.x - current.x;
	nextEdge.y = next.y - current.y;

	/* Normalise the edges */
	normalise(&prevEdge);
	normalise(&nextEdge);

	/* Add them */
	normal.x = prevEdge.x + nextEdge.x;
	normal.y = prevEdge.y + nextEdge.y;

	/* Rotate by 90 degrees */
	rotate(&normal, M_PI / 2.0);

	/* Normalise the result */
	normalise(&normal);

	/* Intersect normal with previous and next edges in the deformed slice */
	currentPlusNormal.x = current.x + normal.x;
	currentPlusNormal.y = current.y + normal.y;
	deformedCurrent.x = current.x + errorCurrent.x;
	deformedCurrent.y = current.y + errorCurrent.y;
	deformedPrev.x = prev.x + errorPrev.x;
	deformedPrev.y = prev.y + errorPrev.y;
	deformedNext.x = next.x + errorNext.x;
	deformedNext.y = next.y + errorNext.y;

	r = intersect(&current, &currentPlusNormal, &deformedPrev, &deformedCurrent, &intersection);
	if(r == SEGMENT_B | r == SEGMENT_BOTH) {
		/* Normal intersected previous deformed edge */
	} else {
		/* Normal missed previous deformed edge, try next */
		r = intersect(&current, &currentPlusNormal, &deformedCurrent, &deformedNext, &intersection);
		if(r == SEGMENT_B | r == SEGMENT_BOTH) {
			/* Normal intersected next deformed edge */
		} else {
			printf("Normal intersected neither edge! point = %d\n", currentPoint);
			fastMode = 0;
		}
	}

	vectorError.x = intersection.x - current.x;
	vectorError.y = intersection.y - current.y;
	scalarError = length(&vectorError);
	output[currentPoint] = scalarError;

	/* Draw point highlights */
	glPointSize(6.0);
	glBegin(GL_POINTS);
	glColor3f(0.0, 0.0, 0.0);
	glVertex2f(current.x, current.y);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(deformedCurrent.x, deformedCurrent.y);
	glColor3f(1.0, 1.0, 0.0);
	glVertex2f(prev.x, prev.y);
	glColor3f(1.0, 0.5, 0.0);
	glVertex2f(next.x, next.y);
	glEnd();

	/* Draw normal */
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex2f(current.x, current.y);
	glVertex2f(currentPlusNormal.x, currentPlusNormal.y);
	glEnd();

	/* Draw intersection point */
	glColor3f(0.0, 0.0, 1.0);
	glPointSize(6.0);
	glBegin(GL_POINTS);
	glVertex2f(intersection.x, intersection.y);
	glEnd();

	/* Draw line between current template point and intersection with deformed slice */
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex2f(current.x, current.y);
	glVertex2f(intersection.x, intersection.y);
	glEnd();
}

void keypress(unsigned char key, int x, int y) {
	switch(key) {
		case 'n':
			if(currentPoint < ((sliceCount * pointsPerSlice) - 1)) {
				currentPoint++;
			}
			glutPostRedisplay();
		break;
		case 'b':
			if(currentPoint > 0) {
				currentPoint--;
			}
			glutPostRedisplay();
		break;
		case ' ':
			fastMode = 1;
			do {
				if(currentPoint < ((sliceCount * pointsPerSlice) - 1)) {
					currentPoint++;
					scalarise();
				} else {
					fastMode = 0;
					if(currentPoint == sliceCount * pointsPerSlice - 1) {
						printf("\nReached end of dataset, press q to write output file and quit.\n");
					}
				}
			} while(fastMode == 1);
			glutPostRedisplay();
		break;
		case '+':
			scope--;
			if(scope < 1) {
				scope = 1;
			}
			glutPostRedisplay();
		break;
		case '-':
			scope++;
			glutPostRedisplay();
		break;
		case 'q':
			writeOutput();
			printf("\n");
			exit(0);
		break;
	}
}

void draw(void) {
	int		i;
	int		currentSlice = currentPoint / pointsPerSlice;

	/* Set up position of our "camera" over the current point */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(template[currentPoint].x - scope, template[currentPoint].x + scope, template[currentPoint].y - scope, template[currentPoint].y + scope, -1.0, 1.0);
	
	/* Wipe the background */
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw whole of current template slice as lines then points */
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	for(i = 0; i < pointsPerSlice; i++) {
		glVertex2f(template[currentSlice * pointsPerSlice + i].x, template[currentSlice * pointsPerSlice + i].y);
	}
	glEnd();
	glPointSize(4.0);
	glBegin(GL_POINTS);
	for(i = 0; i < pointsPerSlice; i++) {
		glVertex2f(template[currentSlice * pointsPerSlice + i].x, template[currentSlice * pointsPerSlice + i].y);
	}
	glEnd();

	/* Draw whole of current deformed slice as lines then points*/
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	for(i = 0; i < pointsPerSlice; i++) {
		glVertex2f(template[currentSlice * pointsPerSlice + i].x + error[currentSlice * pointsPerSlice + i].x, template[currentSlice * pointsPerSlice + i].y + error[currentSlice * pointsPerSlice + i].y);
	}
	glEnd();
	glPointSize(4.0);
	glBegin(GL_POINTS);
	for(i = 0; i < pointsPerSlice; i++) {
		glVertex2f(template[currentSlice * pointsPerSlice + i].x + error[currentSlice * pointsPerSlice + i].x, template[currentSlice * pointsPerSlice + i].y + error[currentSlice * pointsPerSlice + i].y);
	}
	glEnd();

	scalarise();

	glutSwapBuffers();
}

int main(int argc, char *argv[]) {
	FILE		*templateFile, *errorFile;
	char		inputLine[MAXLINELENGTH];
	int		pointsSoFar = 0;
	float		x, y;
	int		i, r;

	if(argc != 4 || strcmp("-h", argv[1]) == 0) {
		printf("Usage: errorscalarise template.sli error.sli output.sli3\nUse b and n to move between points, or spacebar to run without display until an error is found.\nThen press q to exit and write the output file.\nUse - and + to zoom.\n");
		exit(1);
	}

	/* Simple GLUT startup */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Error scalarisor");
	glutKeyboardFunc(keypress);
	glutDisplayFunc(draw);
	glutPostRedisplay();

	/* Read template points */
	templateFile = fopen(argv[1], "r");
	if(templateFile == NULL) {
		printf("Failed to open %s\n", argv[1]);
		exit(1);
	}
	while(fgets(inputLine, MAXLINELENGTH, templateFile)) {
		if(sscanf(inputLine, "slices %d", &sliceCount) == 1) {
			/* Noop */
		} else if(sscanf(inputLine, "points %d", &pointsPerSlice) == 1) {
			template = (vector_t *)malloc(sliceCount * pointsPerSlice * sizeof(vector_t));
		} else if(sscanf(inputLine, "%f %f", &x, &y) == 2) {
			template[pointsSoFar].x = x;
			template[pointsSoFar].y = y;
			pointsSoFar++;
		}
	}
	fclose(templateFile);

	/* Read error points */
	errorFile = fopen(argv[2], "r");
	if(errorFile == NULL) {
		printf("Failed to open %s\n", argv[2]);
		exit(1);
	}
	pointsSoFar = 0;
	error = (vector_t *)malloc(sliceCount * pointsPerSlice * sizeof(vector_t));
	while(fgets(inputLine, MAXLINELENGTH, errorFile)) {
		if(sscanf(inputLine, "%f %f", &x, &y) == 2) {
			error[pointsSoFar].x = x;
			error[pointsSoFar].y = y;
			pointsSoFar++;
		}
	}
	fclose(errorFile);

	outputPath = argv[3];

	/* Conform error function */
	for(i = 0; i < 10; i++) {
		r = conform();
		if(r == 0) {
			break;
		}
		if(i != 9) {
			printf("Conform pass %d ended, made %d changes, conforming again...\n", i + 1, r);
		}
	}
	printf("Conform ended with %d non-aligned points after pass %d.\n", r, i + 1);

	output = (float *)malloc(sliceCount * pointsPerSlice * sizeof(float));
	
	glutMainLoop();
}
