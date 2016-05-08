LINKFLAGS = -L/usr/X11R6/lib -lm -lglut

visualise: main.h main.c maths.c normals.c ui.c colour.c points.c pointnormals.c splats.c sizedsplats.c
	gcc -g main.c maths.c normals.c ui.c colour.c points.c pointnormals.c splats.c sizedsplats.c -o visualise $(LINKFLAGS)

