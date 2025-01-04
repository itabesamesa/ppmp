#include <stdio.h>
#include <stdlib.h>

#include "misc.h"
#include "posterize.h"

My_png div_clip(My_png img, int amount) {
	My_png quantized;
	copy_my_png(&quantized, img);
	pval* quantize = malloc(img.size.x*img.size.y*img.size.z*sizeof(pval));
	int o = 0;
	for(int y = 0; y < img.size.y; y++) {
		for(int x = 0; x < img.size.x; x++) {
			for(int z = 0; z < img.size.z; z++) {
				quantize[o+z] = (pval)(((int)(img.image[o+z]/amount))*amount);
			}
			o += img.size.z;
		}
	}
	quantized.image = quantize;
	return quantized;
}
