#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "misc.h"
#include "noise.h"
#include "scale.h"
#include "my_math.h"

typedef struct _pc {
	xy_int pos;
	pval* color;
} pc;

void noise_n_color(pc* noises, int amount, xyz_int size, int lim) { //check for overlap
	for(int i = 0; i < amount; i++) {
		noises[i].pos.x = (rand()%(size.x+1));
		noises[i].pos.y = (rand()%(size.y+1));
		noises[i].color = malloc(size.z*sizeof(pval));
		lim++;
		for(int z = 0; z < size.z; z++) {
			noises[i].color[z]  = (pval)(rand()%lim);
		}
	}
}

int square(int x) {
	return x*x;
}

int get_color(pc* noises, int amount, int x, int y) {
	int index = 0;
	float value = sqrt(square(noises[0].pos.x-x)+square(noises[0].pos.y-y));
	float tmpval;
	for(int i = 1; i < amount; i++) {
		tmpval = sqrt(square(noises[i].pos.x-x)+square(noises[i].pos.y-y));
		if(tmpval < value) {
			value = tmpval;
			index = i;
		}
	}
	return index;
}

My_png worley_noise_2d(xyz_int size, int cells, unsigned int seed) {
	My_png noised;
	set_my_png(&noised, size, (xy){0, 0}, 0, 0, 0);
	pc noises[cells];
	srand(seed);
	noise_n_color(noises, cells, size, 255);
	pval* noise = malloc(size.y*size.x*size.z*sizeof(pval));
	int o = 0;
	int index;
	for(int y = 0; y < size.y; y++) {
		for(int x = 0; x < size.x; x++) {
			index = get_color(noises, cells, x, y);
			for(int z = 0; z < size.z; z++) {
				noise[o+z] = noises[index].color[z];
			}
			o += size.z;
		}
	}
	noised.image = noise;
	return noised;
}

float smoothstep(float edge0, float edge1, float x) {
	x = clip((x-edge0)/(edge1-edge0), 0, 1);
	return x*x*(3-2*x);
}

float lerp(float a, float b, float c) {
	return a+(b-a)*c;
}

/*pc *grid_n_noise(int grid_size, xyz_int size, int grid_width) {
	int o = 0;
	int offsetx = (size.x%grid_size)/2;
	int offsety = (size.y%grid_size)/2;
	int grid_height = ceil((float)size.y/(float)grid_size);
	//printf("%d %d %d %d %d %d\n", size.x, size.y, grid_width, grid_height, grid_size, grid_width*grid_height);
	pc *noises = malloc(((grid_width)*(grid_height))*sizeof(pc));
	int mallocsize = size.z*sizeof(pval)+1;
	for(int y = -offsety; y < size.y+offsety+(2*grid_size); y += grid_size) {
		//printf("por que\n");
		for(int x = -offsetx; x < size.x+offsetx+(2*grid_size); x += grid_size) {
			noises[o].pos = (xy_int){x, y};
			noises[o].color = malloc(mallocsize);
			for(int z = 0; z < size.z; z++) {
				noises[o].color[z] = 0.5;//((pval)(rand()%255))/255;
			}
			//printf("%d %d %d\n", x, y, o);
			o++;
		}
		//printf("holla\n");
	}
	//printf("asdgsagsafgfd\n");
	return noises;
}*/

My_png grid_n_noise(xyz_int size) {
	int o = 0;
	pval* noises = malloc(size.y*size.x*2*sizeof(pval));
	for(int y = 0; y < size.y; y++) {
		for(int x = 0; x < size.x; x++) {
			noises[o] = ((pval)(rand()%255))/255;
			noises[o+1] = sqrt(1-noises[o]*noises[o]);
			o += 2;
		}
	}
	My_png noiseded;
	initialize_my_png(&noiseded);
	noiseded.size = size;
	noiseded.size.z = 2;
	noiseded.image = noises;
	return noiseded;
}

My_png perlin_noise(xyz_int size, int grid_size, unsigned int seed) {
	My_png noised;
	set_my_png(&noised, size, (xy){0, 0}, 1, 6, 0);
	pval* noise = calloc((size.y*size.x*size.z+3), sizeof(pval));
	xyz_int vector = size;
	vector.z = 2;//redundant
	vector.x = floor(vector.x/grid_size)+1;
	vector.y = floor(vector.y/grid_size)+1;
	//int grid_width = ceil((float)size.x/(float)grid_size);
	//pc *noises = grid_n_noise(grid_size, vector, grid_width);
	My_png noises = grid_n_noise(vector);
	//printf("safwfasfsafasdfsadfdsafdsa\n");
	srand(seed);
	int o = 0;
	int index;
	/*for(int y = 0; y < size.y; y++) {
		for(int x = 0; x < size.x; x++) {
			int i0 = ((int)((y/grid_size)*grid_width))+x/grid_size;
			int i1 = i0+1;
			int i2 = i0+grid_width;
			int i3 = i1+grid_width;
			//printf("x: %d  %d %d %d %d  ", x, i0, i1, i2, i3);
			float d0 = (x-noises[i0].pos.x)*(noises[i0].color[0])+(y-noises[i0].pos.y)*(noises[i0].color[1]);
			float d1 = (x-noises[i1].pos.x)*(noises[i1].color[0])+(y-noises[i1].pos.y)*(noises[i1].color[1]);
			float d2 = (x-noises[i2].pos.x)*(noises[i2].color[0])+(y-noises[i2].pos.y)*(noises[i2].color[1]);
			float d3 = (x-noises[i3].pos.x)*(noises[i3].color[0])+(y-noises[i3].pos.y)*(noises[i3].color[1]);
			pval sum = d0+d1+d2+d3;
			//printf("%.3f %.3f %.3f %d\n", sum, noises[i0].color[0], noises[i0].color[1], noised.size.z);
			for(int z = 0; z < noised.size.z; z++) {
				noise[o+z] = sum;
				//printf("%.3f %.3f ", sum, noise[o+z]);
				o++;
			}
			//printf("\n");
			//o += noised.size.z;
		}
		//printf("y: %d  %d %d %d\n", y, size.x, size.y, size.z);
	}*/
	//printf("holla\n");
	//printf("%d\n", noised.size.z);
	xyz_int selectionsize = {2, 2, 2};
	for(int y = 0; y < size.y; y++) {
		for(int x = 0; x < size.x; x++) {
			xy cord = {(float)x/grid_size, (float)y/grid_size};
			xy cf = {cord.x-floor(cord.x), cord.y-floor(cord.y)};
			xy cc = {cord.x-ceil(cord.x), cord.y-ceil(cord.y)};
			xy selectionstart = {floor(cord.x), floor(cord.y)};
			My_png noisegrid = select_subarray_no_offset(selectionstart, selectionsize, noises);
			float value =
				(lerp(
					lerp(
						cf.x*noisegrid.image[0]+cf.y*noisegrid.image[1],
						cc.x*noisegrid.image[2]+cf.y*noisegrid.image[3],
						cf.x
					),
					lerp(
						cf.x*noisegrid.image[4]+cc.y*noisegrid.image[5],
						cc.x*noisegrid.image[6]+cc.y*noisegrid.image[7],
						cf.x
					),
					cf.y
				)+1)/2;
			for(int z = 0; z < size.z; z++) {
				noise[o+z] = value;
			}
			o += size.z;
		}
	}
	noised.image = noise;
	//My_png noised = bilinear_interpolation(noises, grid_size);
	return noised;
}
