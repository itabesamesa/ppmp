#include <stdlib.h>
#include <stdio.h>

#include "misc.h"
#include "mask.h"

My_png rectangle_mask(xyz_int size, xy_int start, xy_int end) {
	My_png masked;
	set_my_png(&masked, size, (xy){0, 0}, 1, 6, 0);
	pval* mask = calloc(sizeof(pval), size.x*size.y*size.z);
	int starty = clip(start.y, 0, size.y);
	int startx = clip(start.x, 0, size.x);
	int endy = clip(end.y, 0, size.y);
	int endx = clip(end.x, 0, size.x);
	for(int y = starty; y <= endy; y++) {
		int o = (y*size.x+startx)*size.z;
		for(int x = startx; x <= endx; x++) {
			for(int z = 0; z < size.z; z++) {
				mask[o+z] = 1;
			}
			o += size.z;
		}
	}
	masked.image = mask;
	return masked;
}

float sqr(float x) {return x*x;};

My_png circle_mask(xyz_int size, float radius, xy_int pos) {
	My_png masked;
	set_my_png(&masked, size, (xy){0, 0}, 1, 6, 0);
	pval* mask = calloc(sizeof(pval), size.x*size.y*size.z);
	int starty = clip(pos.y-radius, 0, size.y);
	int startx = clip(pos.x-radius, 0, size.x);
	int endy = clip(pos.y+radius, 0, size.y);
	int endx = clip(pos.x+radius, 0, size.x);
	float rsqr = radius*radius;
	for(int y = starty; y <= endy; y++) {
		int o = (y*size.x+startx)*size.z;
		for(int x = startx; x <= endx; x++) {
			if((sqr(x-pos.x)+sqr(y-pos.y)) <= rsqr) {
				for(int z = 0; z < size.z; z++) {
					mask[o+z] = 1;
				}
			}
			o += size.z;
		}
	}
	masked.image = mask;
	return masked;
}

//My_png regular_polygon_mask(xyz_int size, int corners, float radius);

//My_png polygon_mask(xyz_int size, xy_int* corners, int amount);

/*My_png apply_mask(My_png img, My_png mask_img) {
	My_png masked;
	copy_my_png(&masked, img);
	masked.type = 1;
	pval* mask = calloc(sizeof(pval), img.size.x*img.size.y*img.size.z);
	int starty = clip(mask_img.offset.y-img.offset.y, 0, img.size.y);
	int startx = clip(mask_img.offset.x-img.offset.x, 0, img.size.x);
	int endy = clip(start.y+size.y, img.offset.y, img.offset.y+img.size.y)-starty;
	int endx = clip(start.x+size.x, img.offset.x, img.offset.x+img.size.x)-startx;
	float rsqr = radius*radius;
	int o = 0;
	for(int y = starty; y < endy; y++) {
		int oimg = ((y+starty)*img.size.x+startx)*size.z;
		for(int x = startx; x < endx; x++) {
			
			for(int z = 0; z < img.size.z; z++) {
				mask[o+z] *= mask_img.image[oimg+z];
			}
			o += size.z;
			oimg += img.size.z;
		}
	}
	masked.image = mask;
	return masked;
}*/

My_png apply_mask(My_png img, My_png mask, xyz_int size, int img_depth_offset) {
	My_png masked;
	copy_my_png(&masked, img);
	//printf("%d %d %f %f\n", size.x, size.y, img.offset.x, img.offset.y);
	xy_int diffo = {mask.offset.x+img.offset.x, mask.offset.y+img.offset.y};
	xy_int startimg;
	xy_int startmask;
	if(diffo.x < 0) {
		startimg.x = 0;
		startmask.x = -diffo.x;
	} else {
		startimg.x = diffo.x;
		startmask.x = 0;
	}
	if(diffo.y < 0) {
		startimg.y = 0;
		startmask.y = -diffo.y;
	} else {
		startimg.y = diffo.y;
		startmask.y = 0;
	}
	xy diffe = {diffo.x+mask.size.x-img.size.x, diffo.y+mask.size.y-img.size.y};
	xy_int end = {
		(img.size.x-startimg.x < mask.size.x-startmask.x)?img.size.x-startimg.x:mask.size.x-startmask.x,
		(img.size.y-startimg.y < mask.size.y-startmask.y)?img.size.y-startimg.y:mask.size.y-startmask.y
	};
	//printf("%d %d %d %d %d %d\n", startimg.x, startimg.y, startmask.x, startmask.y, end.x, end.y);
	int oimg;
	int omask;
	int depth = (mask.size.z < img.size.z)?mask.size.z:img.size.z;
	pval* maskeded = calloc(sizeof(pval), img.size.x*img.size.y*img.size.z);
	//printf("%d %d %d %d %f %f %f %f\n", mask.size.x, mask.size.y, img.size.x, img.size.y, mask.offset.x, mask.offset.y, img.offset.x, img.offset.y);
	for(int y = 0; y < end.y; y++) {
		for(int x = 0; x < end.x; x++) {
			oimg = ((startimg.y+y)*img.size.x+startimg.x+x)*img.size.z;
			omask = ((startmask.y+y)*mask.size.x+startmask.x+x)*mask.size.z;
			for(int z = 0; z < img.size.z; z++) {
				maskeded[oimg+z] = img.image[oimg+img_depth_offset+z]*mask.image[omask+(z%mask.size.z)];
			}
		}
	}
	masked.image = maskeded;
	return masked;
}

typedef struct _phelper {
	xy_int s, e; //start, end
	float slope;
} phelper;

void selectionSort(int arr[], int n) { //stole from: https://www.geeksforgeeks.org/c-program-to-sort-an-array-in-ascending-order/
	for (int i = 0; i < n-1; i++) {
		int min_idx = i;
		for (int j = i+1; j < n; j++) {
			if (arr[j] < arr[min_idx]) {
				min_idx = j;
			}
		}
		int temp = arr[min_idx];
		arr[min_idx] = arr[i];
		arr[i] = temp;
	}
}

My_png polygon_mask(polygon poly, int depth) {
	phelper* po = malloc(sizeof(phelper)*poly.corners);
	xy_int sb = poly.p[0];
	xy_int bb = poly.p[0];
	for(unsigned int i = 1; i < poly.corners; i++) {
		if(poly.p[i].x < sb.x) {
			sb.x = poly.p[i].x;
		} else if(poly.p[i].x > bb.x) {
			bb.x = poly.p[i].x;
		}
		if(poly.p[i].y < sb.y) {
			sb.y = poly.p[i].y;
		} else if(poly.p[i].y > bb.y) {
			bb.y = poly.p[i].y;
		}
		if(poly.p[i-1].y < poly.p[i].y) {
			po[i].s = poly.p[i-1];
			po[i].e = poly.p[i];
		} else {
			po[i].e = poly.p[i-1];
			po[i].s = poly.p[i];
		}
		if((po[i].e.x-po[i].s.x)) {
			po[i].slope = ((float)(po[i].e.y-po[i].s.y))/((float)(po[i].e.x-po[i].s.x));
		} else {
			po[i].slope = ~(unsigned int)0;
		}
	}
	if(poly.p[poly.corners-1].y < poly.p[0].y) {
		po[0].s = poly.p[poly.corners-1];
		po[0].e = poly.p[0];
	} else {
		po[0].e = poly.p[poly.corners-1];
		po[0].s = poly.p[0];
	}
	if((po[0].e.x-po[0].s.x)) {
		po[0].slope = ((float)(po[0].e.y-po[0].s.y))/((float)(po[0].e.x-po[0].s.x));
	} else {
		po[0].slope = 9999999;
	}

	bb.x++;

	My_png masked;
	set_my_png(&masked, (xyz_int){bb.x-sb.x, bb.y-sb.y, depth}, (xy){sb.x, sb.y}, 1, 6, 0);
	pval* mask = calloc(sizeof(pval), masked.size.x*masked.size.y*depth);

	int intsect[poly.corners];
	int is = 0;

	for(int y = sb.y; y < bb.y; y++) {
		for(unsigned int i = 0; i < poly.corners; i++) {
			if(y >= po[i].s.y && y < po[i].e.y) {
				if(po[i].slope) {
					intsect[is] = (y-po[i].s.y)/po[i].slope+po[i].s.x;
					is++;
				} else {
					intsect[is] = po[i].s.x;
					is++;
					intsect[is] = po[i].e.x;
					is++;
				}
			}
		}
		selectionSort(intsect, is);
		unsigned int oy = (y-sb.y)*masked.size.x*depth;
		for(unsigned int i = 0; i < is-1; i += 2) {
			unsigned int o = oy+(intsect[i]-sb.x)*depth;
			for(int x = intsect[i]; x <= intsect[i+1]; x++) {
				for(unsigned int z = 0; z < depth; z++) mask[o+z] = 1;
				o += depth;
			}
		}
		is = 0;
	}
	masked.image = mask;
	return masked;
}
