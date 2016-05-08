/* Points with normals hedgehog-plot rendering */
#include "main.h"

void initPointnormals(void) {
	glPointSize(1.0);
}

void uninitPointnormals(void) {
	/* Nothing to do here */
}

void renderPointnormals(void) {
	int		i;

	/* Render a point for each vertex */
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_POINTS);
	for(i = 0; i < state.sliceCount * state.pointsPerSlice; i++) {
		glVertex3f(state.data[i].position.x, state.data[i].position.y, state.data[i].position.z);
	}
	glEnd();

	/* Render a green line from each vertex out to five times it's normal vector */
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	for(i = 0; i < state.sliceCount * state.pointsPerSlice; i+=8) {
		glVertex3f(state.data[i].position.x, state.data[i].position.y, state.data[i].position.z);
		glVertex3f(state.data[i].position.x + 5 * state.data[i].normal.x,
			   state.data[i].position.y + 5 * state.data[i].normal.y, 
			   state.data[i].position.z + 5 * state.data[i].normal.z);
	}
	glEnd();
}
