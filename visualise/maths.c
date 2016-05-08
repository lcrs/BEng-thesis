/* Various mathematical utility functions */

#include "main.h"

/* Return length of a vector */
float length(vector3_t v) {
	return(sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z)));
}

/* Normalise a vector */
void normalise(vector3_t *v) {
	float	l;

	l = length(*v);
	v->x /= l;
	v->y /= l;
	v->z /= l;
}

/* Rotate a vector anti-clockwise about the z axis */
void rotateZ(vector3_t *v, float radians) {
	float		x, y;

	x = v->x;
	y = v->y;

	v->x = x * cos(radians) - y * sin(radians);
	v->y = x * sin(radians) + y * cos(radians);
}

/* Return the dot product of two vectors */
float dot(vector3_t a, vector3_t b) {
	return(a.x * b.x + a.y * b.y + a.z * b.z);
}

/* Return the cross product of two vectors */
vector3_t cross(vector3_t a, vector3_t b) {
	vector3_t	v;

	v.x = a.y * b.z - a.z * b.y;
	v.y = a.z * b.x - a.x * b.z;
	v.z = a.x * b.y - a.y * b.x;

	return(v);
}
