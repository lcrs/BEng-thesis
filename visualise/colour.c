/* Colour functions used to convert scalar error value to an RGB colour. */

#include "main.h"

/* Returns RGB colour values as XYZ vector */
vector3_t errorToColour(float error) {
	vector3_t	r;
	int		i;
	float		m, n, f, hue, saturation, value;

	/* Hue is in [0,6] */
	hue = 4.0 - error * 4.0;
	saturation = 1.0;
	value = 0.8;
	
	/* This is now just HSV to RGB conversion */
	i = floor(hue);
	f = hue - i;
	if(i % 2 == 0) {
		/* ...then i is even */
		f = 1 - f;
	}
	m = value * (1 - saturation);
	n = value * (1 - saturation * f);
	switch(i) {
		case 0:
			r.x = value;
			r.y = n;
			r.z = m;
		break;
		case 1:
			r.x = n;
			r.y = value;
			r.z = m;
		break;
		case 2:
			r.x = m;
			r.y = value;
			r.z = n;
		break;
		case 3:
			r.x = m;
			r.y = n;
			r.z = value;
		break;
		case 4:
			r.x = n;
			r.y = m;
			r.z = value;
		break;
		case 5:
			r.x = value;
			r.y = m;
			r.z = n;
		break;
		default:
			r.x = value;
			r.y = n;
			r.z = m;
		break;
	}

	return(r);
}
