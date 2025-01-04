#include <stdio.h>
#include <stdlib.h>

#include "misc.h"

My_png flip_horizontal(My_png img) {
	My_png flipped;
	copy_my_png(&flipped, img);
	pval* flip = malloc(img.size.x*img.size.y*img.size.z*sizeof(pval));
	int o = 0;
	int width = img.size.x*img.size.z;
	int oimg = width;
	width *= 2;
	for(int y = 0; y < img.size.y; y++) {
		for(int x = 0; x < img.size.x; x++) {
			oimg -= img.size.z;
			for(int z = 0; z < img.size.z; z++) {
				flip[o+z] = img.image[oimg+z];
			}
			o += img.size.z;
		}
		oimg += width;
	}
	flipped.image = flip;
	return flipped;
}

My_png flip_vertical(My_png img) {
	My_png flipped;
	copy_my_png(&flipped, img);
	pval* flip = malloc(img.size.x*img.size.y*img.size.z*sizeof(pval));
	int o = 0;
	int oimg = (img.size.y*img.size.x-1)*img.size.z;
	int width = img.size.x*img.size.z*2;
	for(int y = 0; y < img.size.y; y++) {
		for(int x = 0; x < img.size.x; x++) {
			for(int z = 0; z < img.size.z; z++) {
				flip[o+z] = img.image[oimg+z];
			}
			oimg += img.size.z;
			o += img.size.z;
		}
		oimg -= width;
	}
	flipped.image = flip;
	return flipped;
}
