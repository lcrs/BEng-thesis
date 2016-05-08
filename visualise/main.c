/* Startup and setup */
#include "main.h"

/* Size of buffer for reading lines from input file */
#define MAXLINELENGTH 128

/* Read points from input file to array in memory */
void input(char *path) {
	FILE		*f;
	char		inputLine[MAXLINELENGTH];
	float		x, y, e, maxe = 0.0;
	int		i, pointsSoFar = 0;

	f = fopen(path, "r");
	if(f == NULL) {
		printf("Failed to open %s.\n", path);
		exit(1);
	}
	while(fgets(inputLine, MAXLINELENGTH, f)) {
		if(sscanf(inputLine, "slices %d", &state.sliceCount) == 1) {
			/* Noop */
		} else if(sscanf(inputLine, "points %d", &state.pointsPerSlice) == 1) {
			state.data = (point_t *)malloc(state.sliceCount * state.pointsPerSlice * sizeof(point_t));
		} else if(sscanf(inputLine, "%f %f %f", &x, &y, &e) == 3) {
			/* Slice data is read in onto the XY plane, rotated into 3D later */
			state.data[pointsSoFar].position.y = -y;
			state.data[pointsSoFar].position.x = x;
			state.data[pointsSoFar].position.z = 0.0;
			state.data[pointsSoFar].e = e;

			/* Track max e */
			if(e > maxe) {
				maxe = e;
			}

			/* Set the "sample size", which will be used to size splats */
			state.data[pointsSoFar].r = 3.0 * (tan(M_PI / state.sliceCount) / 2.0) * fabs(x) * 2.0;
		
			pointsSoFar++;
		}
	}
	if(pointsSoFar == 0) {
		printf("Failed to read any points.  Are you sure this is a real .sli3 file?\n");
		exit(1);
	}
	fclose(f);

	/* Normalize error values to [0,1], avoiding division by zero */
	if(maxe != 0.0) {
		for(i = 0; i < state.sliceCount * state.pointsPerSlice; i++) {
			state.data[i].e /= maxe;
		}
	}
}

/* Rotate each slice into its proper 3D position */
void rotateSlices(void) {
	float		radians, x;
	int		i;
	
	for(i = 0; i < state.sliceCount * state.pointsPerSlice; i++) {
			radians = (i / state.pointsPerSlice) * (M_PI / state.sliceCount) - M_PI / 2;

			x = state.data[i].position.x;
			state.data[i].position.x = x * cos(radians);
			state.data[i].position.z = x * sin(radians);

			x = state.data[i].normal.x;
			state.data[i].normal.x = x * cos(radians);
			state.data[i].normal.z = x * sin(radians);
			normalise(&state.data[i].normal);
	}
}

/* Window resize callback */
void size(int width, int height) {
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)width / (float)height, 0.1, 1000.0);
}

/* Render method-specific initialisation */
void renderInit(void) {
	switch(state.renderMethod) {
		case POINTS:
			initPoints();
		break;
		case POINTNORMALS:
			initPointnormals();
		break;
		case SPLATS:
			initSplats();
		break;
		case SIZEDSPLATS:
			initSizedSplats();
		break;
	}
	glutPostRedisplay();
}

/* Render method-specific teardown */
void renderUninit(void) {
	switch(state.renderMethod) {
		case POINTS:
			uninitPoints();
		break;
		case POINTNORMALS:
			uninitPointnormals();
		break;
		case SPLATS:
			uninitSplats();
		break;
		case SIZEDSPLATS:
			uninitSizedSplats();
		break;
	}
}

/* Window damage/redraw callback */
void draw(void) {
	vector3_t		up, right;

	/* Clear the window */
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Set up matrices from current state */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(-state.cameraScope, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

	/* Derive camera up and right vectors */
	up.x = 0.0;
	up.y = 0.0;
	up.z = 1.0;

	right.x = -cos(state.cameraAzimuth + (M_PI / 2));
	right.y = sin(state.cameraAzimuth + (M_PI / 2));
	right.z = 0.0;

	/* Rotate and translate dataset to current viewing angle and position */
	glRotatef(state.cameraAzimuth / (M_PI / 180.0), up.x, up.y, up.z);
	glRotatef(-state.cameraElevation / (M_PI / 180.0), right.x, right.y, right.z);

	glTranslatef(-state.cameraX * right.x, -state.cameraX * right.y, -state.cameraX * right.z);
	glTranslatef(-state.cameraY * up.x, -state.cameraY * up.y, -state.cameraY * up.z);

	/* Render dataset */
	switch(state.renderMethod) {
		case POINTS:
			renderPoints();
		break;
		case POINTNORMALS:
			renderPointnormals();
		break;
		case SPLATS:
			renderSplats();
		break;
		case SIZEDSPLATS:
			renderSizedSplats();
		break;
	}

	glutSwapBuffers();
}

int main(int argc, char *argv[]) {
	if(argc != 2 || strcmp("-h", argv[1]) == 0) {
		printf("Usage: visualise input.sli3\nKeys: 1 - Render as point cloud\n      2 - Draw every 8th vertex normal\n      3 - Render as constant-size, screen-aligned, aliased splats\n      4 - Attempt to render as sized, perspective correct, surface-oriented Gaussian splats\n      -/+ to zoom\n      Drag to rotate, right-drag to translate\n      Press q to quit.\n");
		exit(1);
	}

	/* Read data file */
	input(argv[1]);

	/* Derive vertex normals and store with data */
	precalcNormals();

	/* Rotate each slice to its true 3D position */
	rotateSlices();

	/* Simple GLUT startup */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Visualise");

	/* GLUT callback registration */
	glutReshapeFunc(size);
	glutDisplayFunc(draw);
	glutKeyboardFunc(press);
	glutMouseFunc(click);
	glutMotionFunc(drag);
	glutPassiveMotionFunc(move);

	/* Global init */
	state.cameraScope = 300.0;
	state.cameraAzimuth = -M_PI / 4.0;
	state.cameraElevation = M_PI / 8.0;
	state.cameraX = 0.0;
	state.cameraY = 0.0;
	state.dragging = NONE;

	renderInit();

	glutMainLoop();

	return(0);
}
