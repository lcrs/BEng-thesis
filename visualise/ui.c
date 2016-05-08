/* User interface */

#include "main.h"

/* Key press */
void press(unsigned char key, int x, int y) {
	switch(key) {
		case 'q':
			renderUninit();
			exit(0);
		break;
		case '1':
			renderUninit();
			state.renderMethod = POINTS;
			renderInit();
		break;
		case '2':
			renderUninit();
			state.renderMethod = POINTNORMALS;
			renderInit();
		break;
		case '3':
			renderUninit();
			state.renderMethod = SPLATS;
			renderInit();
		break;
		case '4':
			renderUninit();
			state.renderMethod = SIZEDSPLATS;
			renderInit();
		break;
		case '-':
			state.cameraScope += 20;
			if(state.cameraScope > 500) {
				state.cameraScope = 500;
			}
			glutPostRedisplay();
		break;
		case '+':
			state.cameraScope -= 20;
			if(state.cameraScope < 20) {
				state.cameraScope = 20;
			}
			glutPostRedisplay();
		break;
	}
}

/* Mouse click */
void click(int button, int action, int x, int y) {
	switch(action) {
		case GLUT_UP:
			state.dragging = NONE;
		break;
		case GLUT_DOWN:
			state.dragStartX = x;
			state.dragStartY = y;
			switch(button) {
				case GLUT_LEFT_BUTTON:
					state.cameraStartAzimuth = state.cameraAzimuth;
					state.cameraStartElevation = state.cameraElevation;
					state.dragging = ROTATION;
				break;
				case GLUT_RIGHT_BUTTON:
					state.cameraStartX = state.cameraX;
					state.cameraStartY = state.cameraY;
					state.dragging = TRANSLATION;
				break;
			}
	}
}

/* Mouse drag */
void drag(int x, int y) {
	switch(state.dragging) {
		case ROTATION:
			state.cameraAzimuth = state.cameraStartAzimuth + (x - state.dragStartX) / (800.0 / M_PI);
			state.cameraElevation = state.cameraStartElevation + (y - state.dragStartY) / (800.0 / M_PI);
			glutPostRedisplay();
		break;
		case TRANSLATION:
			state.cameraX = state.cameraStartX + (x - state.dragStartX) / 10.0;
			state.cameraY = state.cameraStartY + (y - state.dragStartY) / 10.0;
			glutPostRedisplay();
		break;
	}
}

/* Mouse move */
void move(int x, int y) {
}
