#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "misc.h"
#include "skew.h"

My_png skew_horizontal(My_png img, double angle) {
	angle = -angle;
	double slantx = cos(angle);
	double slanty = sin(angle);
	double k = slantx/slanty;
	int width = floor(abs(img.size.y*k)+img.size.x);
	double d = (slanty < 0)?(width-img.size.x):0;
	My_png skewed;
	copy_my_png(&skewed, img);
	skewed.size.x = width;
	pval* skew = calloc(sizeof(pval), width*img.size.y*img.size.z);
	int o = 0;
	int oimg = 0;
	width *= img.size.z;
	for(int y = 0; y < img.size.y; y++) {
		o = y*width+floor(y*k+d)*img.size.z;
		for(int x = 0; x < img.size.x; x++) {
			for(int z = 0; z < img.size.z; z++) {
				skew[o+z] = img.image[oimg+z];
			}
			o += img.size.z;
			oimg += img.size.z;
		}
	}
	skewed.image = skew;
	return skewed;
}

My_png skew_vertical(My_png img, double angle) {
	angle = -angle;
	double slantx = cos(angle);
	double slanty = sin(angle);
	double k = slantx/slanty;
	printf("%.3f %.3f %.3f\n", slantx, slanty, k);
	int height = floor(abs(img.size.x*k)+img.size.y);
	double d = ((slanty < 0)?(height-img.size.y):0);
	My_png skewed;
	copy_my_png(&skewed, img);
	skewed.size.y = height;
	pval* skew = calloc(sizeof(pval), img.size.x*height*img.size.z);
	int o = 0;
	int oimg = 0;
	int width = img.size.x*img.size.z;

	for(int x = 0; x < img.size.x; x++) {
		//o = y*width+floor(y*k+d)*img.size.z;
		//o = (floor(d)*img.size.x)*img.size.z;
		o = (floor(x*k+d)*img.size.x+x)*img.size.z;
		for(int y = 0; y < img.size.y; y++) {
			for(int z = 0; z < img.size.z; z++) {
				skew[o+z] = img.image[oimg+z];
			}
			//o = floor((d+y)*img.size.x+(img.size.x-x*k))*img.size.z;
			o += width;
			oimg += img.size.z;
		}
	}
	skewed.image = skew;
	return skewed;
}
