#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "misc.h"
#include "blend.h"

pval img_preprocessor(pval value) {
	return (value/255);
}

pval no_processor(pval value) {
	return value;
}

pval img_postprocessor(pval value) {
	return (value*255);
}

pval multiply(pval a, pval b) {
	return a*b;
}

pval screen(pval a, pval b) {
	return 1-((1-a)*(1-b));
}

pval overlay(pval a, pval b) {
	return (a < 0.5)?2*a*b:1-(2*(1-a)*(1-b));
}

pval hard_light(pval a, pval b) {
	return (b < 0.5)?2*a*b:1-(2*(1-a)*(1-b));
}

pval soft_light_photoshop(pval a, pval b) {
	return (b < 0.5)?(2*a*b)+(a*a*(1-2*b)):(2*a*(1-b))+(sqrt(a)*(2*b-1));
}

pval soft_light_pegtop(pval a, pval b) {
	return ((1-2*b)*a*a)+(2*b*a);
}

pval soft_light_illusion_hu(pval a, pval b) {
	return pow(a, pow(2, 2*(0.5-b)));
}

pval soft_light_w3c(pval a, pval b) {
	return (b <= 0.5)?
		a-((1-2*b)*a*(1-a)):
		a+(2*b-1)*(((a <= 0.25)?((16*a-12)*a+4)*a:sqrt(a))-a);
}

pval color_dodge(pval a, pval b) {
	return b/(1-a);
}

pval linear_dodge(pval a, pval b) {
	pval c = a+b;
	return (c > 1)?1:c;
}

pval color_burn(pval a, pval b) {
	return (1-b)/a;
}

pval linear_burn(pval a, pval b) {
	pval c = a+b-1;
	return (c < 0)?0:c;
}

pval vivid_light(pval a, pval b) {
	return a*b; //idk wiki is weird
}

pval linear_light(pval a, pval b) {
	return a*b; //idk wiki is weird
}

pval divide(pval a, pval b) {
	return a/b;
}

pval add(pval a, pval b) {
	pval c = a+b;
	return (c > 1)?1:c;
}

pval subtract(pval a, pval b) {
	pval c = a+b-1;
	return (c < 0)?0:c;
}

pval difference(pval a, pval b) {
	return abs(a-b);
}

pval darken_only(pval a, pval b) {
	return (a < b)?a:b;
}

pval lighten_only(pval a, pval b) {
	return (a < b)?b:a;
}

pval average(pval a, pval b) {
	return (a+b)/2;
}

My_png blend(My_png img1, My_png img2, pval (*blend_func)(pval, pval)) {
	printf("═════════════════════════════\n");
	print_My_png(img1);
	print_My_png(img2);
	printf("═════════════════════════════\n");
	My_png blended;
	xyz_int size;
	if(img1.type > img2.type) {
		copy_my_png(&blended, img2);
		size.z = img2.size.z;
	} else {
		copy_my_png(&blended, img1);
		size.z = img1.size.z;
	}
	My_png images[3] = {img1, img2, (My_png){}};
	xy_int intersection_size;
	if(images[0].offset.x < images[1].offset.x) {
		size.x = images[1].offset.x+images[1].size.x-images[0].offset.x;
		intersection_size.x = images[0].offset.x+images[0].size.x-images[1].offset.x;
	} else {
		size.x = images[0].offset.x+images[0].size.x-images[1].offset.x;
		intersection_size.x = images[1].offset.x+images[1].size.x-images[0].offset.x;
	}
	if(images[0].offset.y < images[1].offset.y) {
		size.y = images[1].offset.y+images[1].size.y-images[0].offset.y;
		intersection_size.y = images[0].offset.y+images[0].size.y-images[1].offset.y;
	} else {
		size.y = images[0].offset.y+images[0].size.y-images[1].offset.y;
		intersection_size.y = images[1].offset.y+images[1].size.y-images[0].offset.y;
		//images = (My_png[2]){images[1], images[0]};
		images[2] = images[0];
		images[0] = images[1];
		images[1] = images[2];
		images[2] = (My_png){};
	}
	unsigned int img2_offset = size.y-images[1].size.y;

	pval (*img1_preprocessor)(pval) = NULL;
	pval (*img2_preprocessor)(pval) = NULL;
	pval (*intersection_postprocessor)(pval) = NULL;
	pval (*img1_postprocessor)(pval) = no_processor;
	pval (*img2_postprocessor)(pval) = no_processor;
	if(images[0].type == IMAGE || images[1].type == IMAGE) {
		intersection_postprocessor = img_postprocessor;
		if(images[0].type) img1_postprocessor = img_postprocessor;
		if(images[1].type) img2_postprocessor = img_postprocessor;
		printf("at least one image\n");
	} else {
		intersection_postprocessor = no_processor;
	}

	if(images[0].type == IMAGE) {
		img1_preprocessor = img_preprocessor;
	} else {
		img1_preprocessor = no_processor;
	}
	if(images[1].type == IMAGE) {
		img2_preprocessor = img_preprocessor;
	} else {
		img2_preprocessor = no_processor;
	}

	blended.size = size;
	pval* blendioned = calloc(size.x*size.y*size.z, sizeof(pval));

	print_My_png(blended);

	unsigned int oimg1 = 0, oimg2 = 0, o = 0;
	if(images[0].offset.x < images[1].offset.x) {
		blended.offset = images[0].offset;
		unsigned int img1padding = (size.x-images[0].size.x)*size.z;
		for(unsigned int y = 0; y < img2_offset; y++) {
			for(unsigned int x = 0; x < images[0].size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = img1_postprocessor(images[0].image[oimg1+(z%images[0].size.z)]);
				}
				o += size.z;
				oimg1 += images[0].size.z;
			}
			o += img1padding;
		}
		for(unsigned int y = 0; y < intersection_size.y; y++) {
			for(unsigned int x = 0; x < images[0].size.x-intersection_size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = img1_postprocessor(images[0].image[oimg1+(z%images[0].size.z)]);
				}
				o += size.z;
				oimg1 += images[0].size.z;
			}
			for(unsigned int x = 0; x < intersection_size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = intersection_postprocessor(
						blend_func(
							img1_preprocessor(images[0].image[oimg1+(z%images[0].size.z)]),
							img2_preprocessor(images[1].image[oimg2+(z%images[1].size.z)])
						)
					);
				}
				o += size.z;
				oimg1 += images[0].size.z;
				oimg2 += images[1].size.z;
			}
			for(unsigned int x = 0; x < images[1].size.x-intersection_size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = img2_postprocessor(images[1].image[oimg2+(z%images[1].size.z)]);
				}
				o += size.z;
				oimg2 += images[1].size.z;
			}
		}
		unsigned int img2padding = (size.x-images[1].size.x)*size.z;
		for(unsigned int y = 0; y < size.y-images[0].size.y; y++) {
			o += img2padding;
			for(unsigned int x = 0; x < images[1].size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = img2_postprocessor(images[1].image[oimg2+(z%images[1].size.z)]);
				}
				o += size.z;
				oimg2 += images[1].size.z;
			}
		}
	} else {
		printf("elsed!!!\n\n\n");
		blended.offset = (xy){images[1].offset.x, images[0].offset.y};
		unsigned int img1padding = (size.x-images[0].size.x)*size.z;
		for(unsigned int y = 0; y < img2_offset; y++) {
			o += img1padding;
			for(unsigned int x = 0; x < images[0].size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = img1_postprocessor(images[0].image[oimg1+(z%images[0].size.z)]);
				}
				o += size.z;
				oimg1 += images[0].size.z;
			}
		}
		printf("finished first loop %d %d %d\n", o, oimg1, oimg2);
		for(unsigned int y = 0; y < intersection_size.y; y++) {
			for(unsigned int x = 0; x < images[1].size.x-intersection_size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = img1_postprocessor(images[1].image[oimg2+(z%images[1].size.z)]);
				}
				o += size.z;
				oimg2 += images[1].size.z;
			}
			printf("%d %d   ", o, oimg2);
			for(unsigned int x = 0; x < intersection_size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = intersection_postprocessor(
						blend_func(
							img1_preprocessor(images[1].image[oimg2+(z%images[1].size.z)]),
							img2_preprocessor(images[0].image[oimg1+(z%images[0].size.z)])
						)
					);
				}
				o += size.z;
				oimg2 += images[1].size.z;
				oimg1 += images[0].size.z;
			}
			printf("%d %d %d   ", o, oimg1, oimg2);
			for(unsigned int x = 0; x < images[0].size.x-intersection_size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = img2_postprocessor(images[0].image[oimg1+(z%images[0].size.z)]);
				}
				o += size.z;
				oimg1 += images[0].size.z;
			}
			printf("%d %d\n", o, oimg1);
		}
		unsigned int img2padding = (size.x-images[1].size.x)*size.z;
		for(unsigned int y = 0; y < size.y-images[0].size.y; y++) {
			for(unsigned int x = 0; x < images[1].size.x; x++) {
				for(unsigned int z = 0; z < size.z; z++) {
					blendioned[o+z] = img2_postprocessor(images[1].image[oimg2+(z%images[1].size.z)]);
				}
				o += size.z;
				oimg2 += images[1].size.z;
			}
			o += img2padding;
		}
	}
	blended.image = blendioned;
	print_My_png(blended);
	return blended;
}

My_png blend_multply(My_png img1, My_png img2) {
	return blend(img1, img2, multiply);
}

My_png blend_screen(My_png img1, My_png img2) {
	return blend(img1, img2, screen);
}

My_png blend_overlay(My_png img1, My_png img2) {
	return blend(img1, img2, overlay);
}

My_png blend_hard_light(My_png img1, My_png img2) {
	return blend(img1, img2, hard_light);
}

My_png blend_soft_light_photoshop(My_png img1, My_png img2) {
	return blend(img1, img2, soft_light_photoshop);
}

My_png blend_soft_light_pegtop(My_png img1, My_png img2) {
	return blend(img1, img2, soft_light_pegtop);
}

My_png blend_soft_light_illusion_hu(My_png img1, My_png img2) {
	return blend(img1, img2, soft_light_illusion_hu);
}

My_png blend_soft_light_w3c(My_png img1, My_png img2) {
	return blend(img1, img2, soft_light_w3c);
}

My_png blend_color_dodge(My_png img1, My_png img2) {
	return blend(img1, img2, color_dodge);
}

My_png blend_linear_dodge(My_png img1, My_png img2) {
	return blend(img1, img2, linear_dodge);
}

My_png blend_color_burn(My_png img1, My_png img2) {
	return blend(img1, img2, color_burn);
}

My_png blend_linear_burn(My_png img1, My_png img2) {
	return blend(img1, img2, linear_burn);
}

My_png blend_vivid_light(My_png img1, My_png img2) {
	return blend(img1, img2, vivid_light);
}

My_png blend_linear_light(My_png img1, My_png img2) {
	return blend(img1, img2, linear_light);
}

My_png blend_divide(My_png img1, My_png img2) {
	return blend(img1, img2, divide);
}

My_png blend_add(My_png img1, My_png img2) {
	return blend(img1, img2, add);
}

My_png blend_subtract(My_png img1, My_png img2) {
	return blend(img1, img2, subtract);
}

My_png blend_difference(My_png img1, My_png img2) {
	return blend(img1, img2, difference);
}

My_png blend_darken_only(My_png img1, My_png img2) {
	return blend(img1, img2, darken_only);
}

My_png blend_lighten_only(My_png img1, My_png img2) {
	return blend(img1, img2, lighten_only);
}

My_png blend_average(My_png img1, My_png img2) {
	return blend(img1, img2, average);
}
