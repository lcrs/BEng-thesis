/* Normal calculation */

#include "main.h"

/* Derive vertex normals by averaging the normals of the surrounding two lines
   Note that this all takes place in 2D on the XY plane, since this is called 
   before the slices are rotated into their true 3D positions */
void precalcNormals(void) {
	int		slice, point;	
	point_t		current, prev, next;
	vector3_t	prevEdge, nextEdge, normal;
	vector3_t	v1, v2, v3, v4;

	for(slice = 0; slice < state.sliceCount; slice++) {
		for(point = 0; point < state.pointsPerSlice; point++) {
			current = state.data[slice * state.pointsPerSlice + point];

			/* Get vectors for previous and next edges relative to current point */
			if(point == 0) {
				/* First point in a slice - previous vertex is last in slice */
				prev = state.data[slice * state.pointsPerSlice + state.pointsPerSlice - 1];
			} else {
				prev = state.data[slice * state.pointsPerSlice + point - 1];
			}
			if(point == state.pointsPerSlice - 1) {
				/* Last point in a slice - next vertex is first in slice */
				next = state.data[slice * state.pointsPerSlice];
			} else {
				next = state.data[slice * state.pointsPerSlice + point + 1];
			}
			prevEdge.x = current.position.x - prev.position.x;
			prevEdge.y = current.position.y - prev.position.y;
			prevEdge.z = 0.0;
			nextEdge.x = next.position.x - current.position.x;
			nextEdge.y = next.position.y - current.position.y;
			nextEdge.z = 0.0;
		
			/* Normalise the edges */
			normalise(&prevEdge);
			normalise(&nextEdge);
		
			/* Add them */
			normal.x = prevEdge.x + nextEdge.x;
			normal.y = prevEdge.y + nextEdge.y;
			normal.z = 0.0;
			
			/* Rotate */
			rotateZ(&normal, -M_PI / 2.0);
			
			/* Normalise the result */
			normalise(&normal);

			/* And store it with the point data */
			current.normal = normal;

			/* Store current point back in the global array */
			state.data[slice * state.pointsPerSlice + point] = current;
		}
	}
}
