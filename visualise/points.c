/* Simplest rendering method - fized-size, screen aligned points */
#include "main.h"

void initPoints(void) {
	glPointSize(1.0);
}

void uninitPoints(void) {
	/* Nothing to do here */
}

void renderPoints(void) {
	int		i;
	vector3_t	c;

	glBegin(GL_POINTS);
	for(i = 0; i < state.sliceCount * state.pointsPerSlice; i++) {
		c = errorToColour(state.data[i].e);
		glColor3f(c.x, c.y, c.z);
		glVertex3f(state.data[i].position.x, state.data[i].position.y, state.data[i].position.z);
	}
	glEnd();
}
