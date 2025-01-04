#include <stdio.h>
#include <stdlib.h>

#include "misc.h"
#include "my_math.h"

My_png generate_box_sample_blur_kernel(int width, int height) {
	My_png kernel;
	set_my_png(&kernel, (xyz_int){width, height, 1}, (xy){0, 0}, 2, 0, 0);
	int arraylength = width*height;
	pval* kernelarray = malloc(arraylength*sizeof(pval));
	pval value = 1./((pval)arraylength);
	for(int i = 0; i < arraylength; i++) {
		kernelarray[i] = value;
	}
	kernel.image = kernelarray;
	return kernel;
}

My_png generate_gaussian_blur_kernel(int width, int height, double sigma) {
	My_png kernel;
	set_my_png(&kernel, (xyz_int){width, height, 1}, (xy){0, 0}, 2, 0, 0);
	int arraylength = width*height;
	pval* kernelarray = malloc(arraylength*sizeof(pval));
	pval value = 1/(pval)arraylength;
	int offsetx = -(width/2);
	int offsety = -(height/2);
	for(int y = 0; y < width; y++) {
		for(int x = 0; x < height; x++) {
			kernelarray[x+y] = (pval)gaussian_function_2d(offsetx+x, offsety+y, sigma);;
		}
	}
	kernel.image = kernelarray;
	return kernel;
}
