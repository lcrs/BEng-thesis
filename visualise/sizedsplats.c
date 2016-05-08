/* Most complex, unfinished render method */
#include "main.h"

/* Attach the coords of a normal-oriented quad to each point */
void setupQuads(void) {
	int		slice, point, i;	
	point_t		current;
	vector3_t	z, axis, v;
	float		dotproduct, theta;
	float		m[16];
	
	for(slice = 0; slice < state.sliceCount; slice++) {
		for(point = 0; point < state.pointsPerSlice; point++) {
			current = state.data[slice * state.pointsPerSlice + point];
			
			/* Set up a unit quad on the XY plane */
			current.quad[0].x = -0.5;
			current.quad[0].y = 0.5;
			current.quad[0].z = 0.0;

			current.quad[1].x = 0.5;
			current.quad[1].y = 0.5;
			current.quad[1].z = 0.0;

			current.quad[2].x = 0.5;
			current.quad[2].y = -0.5;
			current.quad[2].z = 0.0;

			current.quad[3].x = -0.5;
			current.quad[3].y = -0.5;
			current.quad[3].z = 0.0;

			/* Size this quad to the radius of the point */
			current.quad[0].x *= current.r;
			current.quad[0].y *= current.r;
			current.quad[1].x *= current.r;
			current.quad[1].y *= current.r;
			current.quad[2].x *= current.r;
			current.quad[2].y *= current.r;
			current.quad[3].x *= current.r;
			current.quad[3].y *= current.r;

			/* Rotate this quad to face it along the normal vector: */
			/* Take the dot product of the current normal (the z axis) and the desired normal */
			z.x = 0.0;
			z.y = 0.0;
			z.z = 1.0;
			dotproduct = dot(z, current.normal);

			/* Hence find the angle between the two */
			if(dotproduct >= 1.0) {
				/* Already pointing along normal */
				theta = 0.0;
			} else {
				theta = acosf(dotproduct);
			}

			/* Take the cross product of the current normal (the z axis) and the desired normal, to get an axis of rotation */
			axis = cross(z, current.normal);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();

			/* For each vertex: */
			for(i = 0; i < 4; i++) {
				/* Setup up a GL rotation matrix and extract it to the array m */
				glLoadIdentity();
				glRotatef(theta * (180.0 / M_PI), axis.x, axis.y, axis.z);
				glGetFloatv(GL_MODELVIEW_MATRIX, m);

				/* Multiply vertex coords by this rotation matrix */
				v = current.quad[i];

				current.quad[i].x = v.x * m[0] + v.y * m[4] + v.z * m[8];
				current.quad[i].y = v.x * m[1] + v.y * m[5] + v.z * m[9];
				current.quad[i].z = v.x * m[2] + v.y * m[6] + v.z * m[10];
			}

			glPopMatrix();

			/* Store the current point back in the global array */
			state.data[slice * state.pointsPerSlice + point] = current;
		}
	}
}

/* Create a fuzzy Gaussian blob radial gradient texture for drawing splats */
void setupTexture(void) {
	float		*g;
	float		x, y;
	int		i, j, t;

	g = (float *)malloc(64 * 64 * sizeof(float));

	for(i = 0; i < 64; i++) {
		for(j = 0; j < 64; j++) {
			x = 3.0 * ((float)i - 32.0) / 32.0;
			y = 3.0 * ((float)j - 32.0) / 32.0;
			g[i * 64 + j] = (1.0 / 2.0 * M_PI) * expf(-(x * x + y * y ) / 2.0);
		}
	}

	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_FLOAT, g);

	free(g);
}

void initSizedSplats(void) {
	int		i;
	point_t		p;
	vector3_t	c;
	float		l[4] = {-25.0, -50.0, 50.0, 0.0};

	setupQuads();
	setupTexture();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glLightfv(GL_LIGHT0, GL_POSITION, l);

	glEnable(GL_TEXTURE_2D);

	/* Build a display list for the whole dataset for faster drawing */
	state.displayList = glGenLists(1);
	glNewList(state.displayList, GL_COMPILE);
	glBegin(GL_QUADS);
	for(i = 0; i < state.pointsPerSlice * state.sliceCount; i++) {
		p = state.data[i];
		c = errorToColour(p.e);
		glColor4f(c.x, c.y, c.z, 1.0);
		glNormal3f(p.normal.x, p.normal.y, p.normal.z);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(p.position.x + p.quad[0].x, p.position.y + p.quad[0].y, p.position.z + p.quad[0].z);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(p.position.x + p.quad[1].x, p.position.y + p.quad[1].y, p.position.z + p.quad[1].z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(p.position.x + p.quad[2].x, p.position.y + p.quad[2].y, p.position.z + p.quad[2].z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(p.position.x + p.quad[3].x, p.position.y + p.quad[3].y, p.position.z + p.quad[3].z);
	}
	glEnd();
	glEndList();
}

void uninitSizedSplats(void) {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);
}

void renderSizedSplats(void) {
	/* First, draw the model accumulating the closest Z depths (offset back slightly), without updating the colour buffers */
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, 200.0);
	glCallList(state.displayList);

	/* Now draw it again, updating the colour buffers only where the incoming depth is closer than the stored depth */
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_FALSE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glCallList(state.displayList);

	/* Re-enable depth updates for glClear() in draw() in main.c */
	glDepthMask(GL_TRUE);
}
