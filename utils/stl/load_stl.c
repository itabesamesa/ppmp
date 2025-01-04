#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "load_stl.h"

void get_vertex(FILE *fp, float *coord) {
	char tmp[4];
	fgets(tmp, 4, fp);
	memcpy(&coord[0], tmp, 4);
	fgets(tmp, 4, fp);
	memcpy(&coord[1], tmp, 4);
	fgets(tmp, 4, fp);
	memcpy(&coord[2], tmp, 4);
	//printf("%f %f %f\n", coord[0], coord[1], coord[2]);
}

triangle get_triag(FILE *fp) {
	triangle triag;
	char tmp[4];
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.normal[0], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.normal[1], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.normal[2], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.vertex1[0], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.vertex1[1], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.vertex1[2], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.vertex2[0], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.vertex2[1], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.vertex2[2], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.vertex3[0], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&triag.vertex3[1], tmp, 4);
	fgets(tmp, 4, fp);
	//printf("%.3f ", tmp);
	memcpy(&((triag.vertex3)[2]), tmp, 4);
	char tmp2[2];
	fgets(tmp2, 2, fp);
	memcpy(&(triag.attribute), tmp2, 2);
	//printf("%d\n", triag.attribute);
	return triag;
}

stl openstl(char *file) {
	printf("Using file: %s\n", file);
	FILE *fp = fopen(file, "rb");
	stl geom;
	fgets(geom.header, 81, fp);
	unsigned char amount[4];
	fgets(amount, 4, fp);
	geom.amount = (unsigned long)(amount[0]+(amount[1]<<8)+(amount[2]<<16)+(amount[3]<<24));
	//printf("header: %s\namount: %u|%x|%x|%x|%x|\n", geom.header, geom.amount, amount[3], amount[2], amount[1], amount[0]);
	geom.faces = malloc(geom.amount*sizeof(triangle));
	for(int i = 0; i < geom.amount; i++) {
		geom.faces[i] = get_triag(fp);
		//printf("%x %x %x %x %x %x %x %x %x\n", geom.faces[i].normal[0], geom.faces[i].normal[1], geom.faces[i].normal[2], geom.faces[i].vertex1[0], geom.faces[i].vertex1[1], geom.faces[i].vertex1[2], geom.faces[i].vertex2[0], geom.faces[i].vertex2[1], geom.faces[i].vertex2[2], geom.faces[i].vertex3[0], geom.faces[i].vertex3[1], geom.faces[i].vertex3[2]);
	}
	return geom;
}
