/* Constant-size splats render method */
#include "main.h"

void initSplats(void) {
	float		p[4] = {-25.0, -50.0, 50.0, 0.0};
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	glLightfv(GL_LIGHT0, GL_POSITION, p);

	glPointSize(16.0);

	glEnable(GL_POINT_SMOOTH);
	/* Clearly blending is required for overlapping smooth points but naive z-buffering breaks it.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
}

void uninitSplats(void) {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_COLOR_MATERIAL);
}

void renderSplats(void) {
	int		i;
	vector3_t	c;
	
	glBegin(GL_POINTS);
	for(i = 0; i < state.sliceCount * state.pointsPerSlice; i++) {
		c = errorToColour(state.data[i].e);
		glColor3f(c.x, c.y, c.z);
		glNormal3f(state.data[i].normal.x, state.data[i].normal.y, state.data[i].normal.z);
		glVertex3f(state.data[i].position.x, state.data[i].position.y, state.data[i].position.z);
	}
	glEnd();
}
