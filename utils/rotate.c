#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "misc.h"
#include "skew.h"
#include "rotate.h"

My_png rotate_90(My_png img) {
	My_png rotated;
	copy_my_png(&rotated, img);
	rotated.size = (xyz_int){img.size.y, img.size.x, img.size.z};
	pval* rotate = malloc(sizeof(pval)*img.size.x*img.size.y*img.size.z);
	int o = 0;
	int oimg = 0;
	int width = img.size.y*img.size.z;
	for(int y = img.size.y-1; y >= 0; y--) {
		o = y*img.size.z;
		for(int x = 0; x < img.size.x; x++) {
			for(int z = 0; z < img.size.z; z++) {
				rotate[o+z] = img.image[oimg+z];
			}
			o += width;
			oimg += img.size.z;
		}
	}
	rotated.image = rotate;
	return rotated;
}

My_png rotate_180(My_png img) {
	My_png rotated;
	copy_my_png(&rotated, img);
	int o = img.size.x*img.size.y*img.size.z;
	pval* rotate = malloc(sizeof(pval)*o);
	int oimg = 0;
	for(int y = img.size.y-1; y >= 0; y--) {
		for(int x = 0; x < img.size.x; x++) {
			o -= img.size.z;
			for(int z = 0; z < img.size.z; z++) {
				rotate[o+z] = img.image[oimg+z];
			}
			oimg += img.size.z;
		}
	}
	rotated.image = rotate;
	return rotated;
}

My_png rotate_270(My_png img) {
	My_png rotated;
	copy_my_png(&rotated, img);
	rotated.size = (xyz_int){img.size.y, img.size.x, img.size.z};
	pval* rotate = malloc(sizeof(pval)*img.size.x*img.size.y*img.size.z);
	int o = 0;
	int oimg = 0;
	int width = img.size.y*img.size.z;
	for(int y = 0; y < img.size.y; y++) {
		o = y*img.size.z;
		for(int x = 0; x < img.size.x; x++) {
			for(int z = 0; z < img.size.z; z++) {
				rotate[o+z] = img.image[oimg+z];
			}
			o += width;
			oimg += img.size.z;
		}
	}
	rotated.image = rotate;
	return rotated;
}

My_png rotate_skew(My_png img, double angle) {
	My_png skew1 = skew_horizontal(img, -angle/2);
	My_png skew2 = skew_vertical(skew1, angle);
	free(skew1.image);
	My_png skew3 = skew_horizontal(skew2, -angle/2);
	free(skew2.image);
	return skew3;
}

xy rotate_2d(double ca, double sa, xy p) {
	xy pr;
	pr.x = (p.x*ca)-(p.y*sa);
	pr.y = (p.x*sa)+(p.y*ca);
	return pr;
}

float mapRange(float start1, float end1, float start2, float end2, float value) {
	return ((end1-start1)/(end2-start2))*value+start2;
}

My_png rotate_rotation_matrix(My_png img, double angle) {
	angle = -angle;
	float ox = ((float)img.size.x)/2;
	float oy = ((float)img.size.y)/2;
	float ca = cos(angle);
	float sa = sin(angle);
	xy cr1 = rotate_2d(ca, sa, (xy){img.size.x, 0});
	xy cr2 = rotate_2d(ca, sa, (xy){img.size.x, img.size.y});
	xy cr3 = rotate_2d(ca, sa, (xy){0, img.size.y});
	int height;
	int width;
	if(abs(cr2.y) >= img.size.y) {
		height = abs(cr2.y);
		width = abs(cr1.x)+abs(cr3.x);
	} else {
		height = abs(cr1.y)+abs(cr3.y);
		width = abs(cr2.x);
	}
	ca = cos(angle);
	sa = sin(angle);
	My_png rotated;
	copy_my_png(&rotated, img);
	rotated.size = (xyz_int){width, height, img.size.z};
	pval* rotate = calloc(sizeof(pval), ceil((width+20)*(height+20)*img.size.z));
	int o = 0;
	xyz_int selectionsize = {2, 2, img.size.z};
	xy halfsize = {rotated.size.x/2, rotated.size.y/2};
	for(int y = 0; y < rotated.size.y; y++) {
		for(int x = 0; x < rotated.size.x; x++) {
			xy cord = {x-halfsize.x, y-halfsize.y};
			xy cordr = rotate_2d(ca, sa, cord);
			if((cordr.x > -ox || cordr.x < ox) && (cordr.y > -oy || cordr.y < oy)) {
				xy cf = {cordr.x-floor(cordr.x), cordr.y-floor(cordr.y)};
				xy cc = {cordr.x-ceil(cordr.x), cordr.y-ceil(cordr.y)};
				float weights[4] = {
					sqrt(cf.x*cf.x+cf.y*cf.y),
					sqrt(cc.x*cc.x+cf.y*cf.y),
					sqrt(cf.x*cf.x+cc.y*cc.y),
					sqrt(cc.x*cc.x+cc.y*cc.y)
				};
				float total = weights[0]+weights[1]+weights[2]+weights[3];
				xy selectionstart = {floor(cordr.x)+ox, floor(cordr.y)+oy};
				My_png boxything = select_subarray_no_offset(selectionstart, selectionsize, img);
				for(int z = 0; z < rotated.size.z; z++) {
					rotate[o+z] =
						boxything.image[z]*(weights[0]/total)+
						boxything.image[boxything.size.z+z]*(weights[1]/total)+
						boxything.image[boxything.size.z*2+z]*(weights[2]/total)+
						boxything.image[boxything.size.z*3+z]*(weights[3]/total);
				}
			}
			o += rotated.size.z;
		}
	}
	rotated.image = rotate;
	return rotated;
}
