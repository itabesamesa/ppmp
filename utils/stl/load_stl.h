#ifndef LOAD_STL_H
#define LOAD_STL_H

typedef struct _triangle {
	float normal[3], vertex1[3], vertex2[3], vertex3[3];
	unsigned int attribute;
} triangle;

typedef struct _stl {
	char header[81];
	unsigned long amount;
	triangle *faces;
} stl;

stl openstl(char *file);

#endif
