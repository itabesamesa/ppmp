#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <math.h>
#include <string.h>

#include "misc.h"
#include "pngtopng.h"

void add2DVector(xy v, xy *v2) {
	v2->x = v.x+v2->x;
	v2->y = v.y+v2->y;
}

void sub2DVector(xy v1, xy v2, xy *r) {
	r->x = v1.x-v2.x;
	r->y = v1.y-v2.y;
}

void scale2DVector(float s, xy *v) {
	v->x = v->x*s;
	v->y = v->y*s;
}

float clip(float value, float min, float max) {
	if (value < min) {
		return min;
	} else if (value > max) {
		return max;
	} else {
		return value;
	}
}

pval max(pval* l, int amount) {
	pval max_value = l[0];
	for(int i = 1; i < amount; i++) {
		if(max_value < l[i]) {
			max_value = l[i];
		}
	}
	return max_value;
}

pval min(pval* l, int amount) {
	pval min_value = l[0];
	for(int i = 1; i < amount; i++) {
		if(min_value > l[i]) {
			min_value = l[i];
		}
	}
	return min_value;
}

int color_to_depth(unsigned short type) {
	switch(type) {
		case 0:
		case 2:
		case 4:
			return 3;
		case 1:
		case 3:
		case 5:
			return 4;
		case 6:
			return 1;
		default:
			return 0;
	}
}

color color_to_color(unsigned short type, color c) {
	switch(type*6+c.type) {}
	return c;
}

pval bw(pval* l, int amount) {
	pval value = l[0];
	for(int i = 1; i < amount; i++) {
		value += l[i];
	}
	return (value/((pval)amount));
}

void initialize_my_png(My_png* img) {
	img->size = (xyz_int){0, 0, 0};
	img->offset = (xy){0, 0};
	img->type = 0;
	img->color_type = 0;
	img->display_type = 0;
}

void set_my_png(My_png* img, xyz_int size, xy offset, short type, short color_type, short display_type) {
	img->size = size;
	img->offset = offset;
	img->type = type;
	img->color_type = color_type;
	img->display_type = display_type;
}

void copy_my_png(My_png* img1, My_png img2) {
	img1->size = img2.size;
	img1->offset = img2.offset;
	img1->type = img2.type;
	img1->color_type = img2.color_type;
	img1->display_type = img2.display_type;
}

My_png select_subarray(xy start, xyz_int size, My_png img) {
	My_png array;
	copy_my_png(&array, img);
	array.size = size;
	array.offset = start;
	pval* subarray = calloc(sizeof(pval), size.x*size.y*size.z);
	int starty = clip(start.y, img.offset.y, img.size.y); //what if img.offset is bigger than img.size?
	int startx = clip(start.x, img.offset.x, img.size.x);
	int endy = clip(start.y+size.y, img.offset.y, img.offset.y+img.size.y)-starty;
	int endx = clip(start.x+size.x, img.offset.x, img.offset.x+img.size.x)-startx;
	//int offsety = start.y-img.offset.y;
	//offsety = (offsety < 0) ? -offsety : 0;
	//int offsetx = start.x-img.offset.x;
	//offsetx = (offsetx < 0) ? (-offsetx) : 0;
	int o = 0;
	for(int y = 0; y < endy; y++) {
		int oimg = ((y+starty)*img.size.x+startx)*size.z;
		for(int x = 0; x < endx; x++) {
			for(int z = 0; z < img.size.z; z++) {
				subarray[o+z] = img.image[oimg+z];
			}
			o += size.z;
			oimg += img.size.z;
		}
	}
	array.image = subarray;
	return array;
}

My_png select_subarray_no_offset(xy start, xyz_int size, My_png img) {
	My_png array;
	copy_my_png(&array, img);
	array.size = size;
	pval* subarray = calloc(sizeof(pval), size.x*size.y*size.z);
	int starty = clip(start.y, 0, img.size.y);
	int startx = clip(start.x, 0, img.size.x);
	int endy = clip(start.y+size.y, 0, img.offset.y+img.size.y)-starty;
	int endx = clip(start.x+size.x, 0, img.offset.x+img.size.x)-startx;
	int o = 0;
	for(int y = 0; y < endy; y++) {
		int oimg = ((y+starty)*img.size.x+startx)*size.z;
		for(int x = 0; x < endx; x++) {
			for(int z = 0; z < img.size.z; z++) {
				subarray[o+z] = img.image[oimg+z];
			}
			o += size.z;
			oimg += img.size.z;
		}
	}
	array.image = subarray;
	return array;
}

void accumulate_convolute_kernel(My_png kernel, My_png img, pval* accumulator) {
	xyz_int size = kernel.size;
	size.z = img.size.z;
	My_png subimg = select_subarray(kernel.offset, size, img);
	int o = 0;
	int ok = 0;
	for(int y = 0; y < kernel.size.y; y++) {
		for(int x = 0; x < kernel.size.x; x++) {
			for(int z = 0; z < img.size.z; z++) {
				accumulator[z] += subimg.image[o+z]*kernel.image[ok];
			}
			o += img.size.z;
			ok += kernel.size.z;
		}
	}
}

My_png convolute_img(My_png kernel, My_png img) {
	My_png convoluted;
	copy_my_png(&convoluted, img);
	pval* convolutioned = malloc(img.size.x*img.size.y*img.size.z*sizeof(pval));
	float kernel_offsetx = (float)kernel.size.x/2;
	float kernel_offsety = (float)kernel.size.y/2;
	//xy offset = kernel.offset;
	kernel.offset = (xy){-kernel_offsetx, -kernel_offsety};
	img.offset = (xy){0, 0};
	int o = 0;
	for(int y = 0; y < img.size.y; y++) {
		for(int x = 0; x < img.size.x; x++) {
			pval* accumulator = calloc(sizeof(pval), img.size.z);
			accumulate_convolute_kernel(kernel, img, accumulator);
			for(int z = 0; z < img.size.z; z++) {
				convolutioned[o+z] = accumulator[z];
			}
			o += img.size.z;
			kernel.offset.x++;
		}
		kernel.offset.x = -kernel_offsetx;
		kernel.offset.y++;
	}
	convoluted.image = convolutioned;
	//kernel.offset = offset;
	return convoluted;
}

pval* accumulate_convolute_multiple_kernels(My_png** kernels, int amount, My_png img, xy pos) {
	pval* accumulators = calloc(sizeof(pval), img.size.z*amount);
	xyz_int size = kernels[0]->size;
	size.z = img.size.z;
	My_png subimg = select_subarray(pos, size, img);
	int o = 0;
	int ok = 0;
	for(int y = 0; y < kernels[0]->size.y; y++) {
		for(int x = 0; x < kernels[0]->size.x; x++) {
			for(int i = 0; i < amount; i++) {
				for(int z = 0; z < img.size.z; z++) {
					accumulators[i*img.size.z+z] += subimg.image[o+z]*kernels[i]->image[ok];
				}
			}
			o += img.size.z;
			ok += kernels[0]->size.z;
		}
	}
	return accumulators;
}

My_png convolute_img_multiple_kernels(My_png** kernels, int amount, My_png img, void ((*accumulator_func)(pval*, pval*, int, int))) {
	My_png convoluted;
	copy_my_png(&convoluted, img);
	pval* convolutioned = malloc(img.size.x*img.size.y*img.size.z*sizeof(pval));
	float kernel_offsetx = (float)kernels[0]->size.x/2;
	float kernel_offsety = (float)kernels[0]->size.y/2;
	convoluted.offset = img.offset;
	img.offset = (xy){0, 0};
	int o = 0;
	for(float y = 0; y < img.size.y; y++) {
		for(float x = 0; x < img.size.x; x++) {
			xy pos = {x-kernel_offsetx, y-kernel_offsety};
			pval* accumulators = accumulate_convolute_multiple_kernels(kernels, amount, img, pos);
			pval* accumulator = malloc(img.size.z*sizeof(pval));
			(*accumulator_func)(accumulators, accumulator, img.size.z, amount);
			for(int z = 0; z < img.size.z; z++) {
				convolutioned[o+z] = accumulator[z];
			}
			o += img.size.z;
		}
	}
	convoluted.image = convolutioned;
	return convoluted;
}

void plonk(My_png img, int x, int y) {
	if((x >= 0 || x < img.size.x) && (y >= 0 || y < img.size.y)) {
		int o = (y*img.size.x+x)*img.size.z;
		for(int z = 0; z < img.size.z; z++) {
			img.image[o] = 255;
		}
	}
}

My_png empty_img(int width, int height, int depth) {
	My_png img;
	set_my_png(&img, (xyz_int){width, height, depth}, (xy){0, 0}, 0, 0, 0);
	pval* empty = calloc(sizeof(pval), width*height*depth);
	img.image = empty;
	return img;
}

My_png empty_colored_img(int width, int height, int depth, pval* color) {
	My_png img;
	set_my_png(&img, (xyz_int){width, height, depth}, (xy){0, 0}, 0, 0, 0);
	pval* empty = malloc(sizeof(pval)*width*height*depth);
	int o = 0;
	for(float y = 0; y < height; y++) {
		for(float x = 0; x < width; x++) {
			for(int z = 0; z < depth; z++) {
				empty[o+z] = color[z];
			}
			o += depth;
		}
	}
	img.image = empty;
	return img;
}

polygon regular_plygon(unsigned int corners, unsigned int radius, double angle) {
	polygon p;
	p.corners = corners;
	p.p = malloc(corners*sizeof(xy_int));
	double ao = M_PI*2/corners;
	for(unsigned int i = 0; i < corners; i++) {
		p.p[i] = (xy_int){cos(angle)*radius, sin(angle)*radius};
		angle += ao;
	}
	return p;
}

My_png move_img(My_png img, xy pos) {
	My_png moved;
	copy_my_png(&moved, img);
	moved.offset = pos;
	size_t len = img.size.x*img.size.y*img.size.z*sizeof(pval);
	moved.image = malloc(len);
	memcpy(moved.image, img.image, len);
	return moved;
}

void test_plonk(My_png img) {
	float ca = cos(0.5);
	float sa = sin(0.5);
	for(int y = 0; y < 20; y++) {
		//plonk(img, y, y*y*0.05);
		for(int x = 0; x < 20; x++) {
			int cx = (x*ca)-(y*sa);
			int cy = (x*sa)+(y*ca);
			plonk(img, cx+20, cy+20);
		}
	}
}

short decode_color(char* color) {
	
}

void print_My_png(My_png img) {
	printf("image:\n");
	printf("\tsize:\n");
	printf("\t\tx: %d\n\t\ty: %d\n\t\tz: %d\n", img.size.x, img.size.y, img.size.z);
	printf("\toffset:\n");
	printf("\t\tx: %.3f\n\t\ty: %.3f\n", img.offset.x, img.offset.y);
	printf("\ttype: %d\n", img.type);
	printf("\tcolor type: %d\n", img.color_type);
	printf("\tdisplay type: %d\n", img.display_type);
}
