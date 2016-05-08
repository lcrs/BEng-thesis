/* Global header */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glut.h>

/* Simple 3D vector type for convenience */
typedef struct vector3_t {
	float	x;
	float	y;
	float	z;
} vector3_t;

/* Type for points in our dataset */
typedef struct point_t {
	vector3_t	position;
	vector3_t	normal;
	float		e;		/* Per-point error value */
	float		r;		/* Size of sample, radius of splat */
	vector3_t	quad[4];	/* Coords of a normal-aligned quad */
} point_t;

/* Type for current rendering method */
typedef enum renderMethod_t {
	POINTS,
	POINTNORMALS,
	SPLATS,
	SIZEDSPLATS
} renderMethod_t;

/* Type for current mouse drag operation */
typedef enum dragging_t {
	NONE,
	ROTATION,
	TRANSLATION
} dragging_t;

/* Maths utils */
float length(vector3_t v);
void normalise(vector3_t *v);
void rotateZ(vector3_t *v, float radians);
float dot(vector3_t a, vector3_t b);
vector3_t cross(vector3_t a, vector3_t b);

/* Misc prototypes */
void precalcNormals(void);
void renderInit(void);
void renderUninit(void);
vector3_t errorToColour(float error);

/* GLUT event callback prototypes */
void size(int width, int height);
void draw(void);
void press(unsigned char key, int x, int y);
void click(int button, int state, int x, int y);
void drag(int x, int y);
void move(int x, int y);

/* Render method prototypes */
void initPoints(void);
void uninitPoints(void);
void renderPoints(void);
void initPointnormals(void);
void uninitPointnormals(void);
void renderPointnormals(void);
void initSplats(void);
void uninitSplats(void);
void renderSplats(void);
void initSizedSplats(void);
void uninitSizedSplats(void);
void renderSizedSplats(void);

/* Global state */
typedef struct state_t {
	/* Current rendering method */
	renderMethod_t	renderMethod;

	/* Array for our dataset */
	point_t		*data;

	/* Properties of the dataset */
	int		sliceCount;
	int		pointsPerSlice;

	/* State variables for mouse interaction */
	dragging_t	dragging;
	int		dragStartX, dragStartY;

	/* Camera state */
	float		cameraScope, cameraAzimuth, cameraElevation, cameraX, cameraY;

	/* Camera state at begining of drag */
	float		cameraStartAzimuth, cameraStartElevation, cameraStartX, cameraStartY;

	/* Number of display list used by renderer */
	int		displayList;
} state_t;
state_t	state;
