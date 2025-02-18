#include <png.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "misc.h"
#include "pngtopng.h"

My_png openimg(char* file) {
	int depth = 4;

	//this too
	//**
	int width, height;
	png_byte color_type;
	png_byte bit_depth;
	png_bytep* row_pointers = NULL;
	//**

	//stole from here "https://gist.github.com/niw/5963798"
	//********************************
	FILE* fp = fopen(file, "rb");

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) abort();

	png_infop info = png_create_info_struct(png);
	if(!info) abort();

	if(setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	png_read_info(png, info);

	width      = png_get_image_width(png, info);
	height     = png_get_image_height(png, info);
	color_type = png_get_color_type(png, info);
	bit_depth  = png_get_bit_depth(png, info);

	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if(bit_depth == 16)
	png_set_strip_16(png);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
	png_set_palette_to_rgb(png);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	png_set_expand_gray_1_2_4_to_8(png);

	if(png_get_valid(png, info, PNG_INFO_tRNS))
	png_set_tRNS_to_alpha(png);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if(color_type == PNG_COLOR_TYPE_RGB ||
		color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if(color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);

	if (row_pointers) abort();

	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) {
		row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	png_read_image(png, row_pointers);

	fclose(fp);

	png_destroy_read_struct(&png, &info, NULL);
	//********************************

	My_png img;
	initialize_my_png(&img);

	pval* h = malloc(height*width*depth*sizeof(pval));
	for(int y = 0; y < height; y++) {
		png_bytep row = row_pointers[y];
		for(int x = 0; x < width; x++) {
			int o = (y*width+x)*depth;
			png_bytep px = &(row[x * depth]);
			h[o] = px[0];     //maybe add conversion to pval
			h[o+1] = px[1];
			h[o+2] = px[2];
			h[o+3] = px[3];
		}
	}
	img.image = h;
	img.size = (xyz_int){width, height, depth};

	return img;
}

void save_image(My_png img, char* path) {
	FILE * fp;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	size_t x, y;
	png_byte ** row_pointers = NULL;
	/* "status" contains the return value of this function. At first
		 it is set to a value which means 'failure'. When the routine
		 has finished its work, it is set to a value which means
		 'success'. */
	int status = -1;
	/* The following number is set by trial and error only. I cannot
		 see where it it is documented in the libpng manual.
	*/
	int pixel_size = 3;
	int depth = 8;
	
	fp = fopen (path, "wb");
	if (! fp) {
			goto fopen_failed;
	}

	png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
			goto png_create_write_struct_failed;
	}
	
	info_ptr = png_create_info_struct (png_ptr);
	if (info_ptr == NULL) {
			goto png_create_info_struct_failed;
	}
	
	/* Set up error handling. */

	if (setjmp (png_jmpbuf (png_ptr))) {
			goto png_failure;
	}
	
	/* Set image attributes. */

	png_set_IHDR (png_ptr,
								info_ptr,
								img.size.x,
								img.size.y,
								depth,
								PNG_COLOR_TYPE_RGB,
								PNG_INTERLACE_NONE,
								PNG_COMPRESSION_TYPE_DEFAULT,
								PNG_FILTER_TYPE_DEFAULT);
	
	/* Initialize rows of PNG. */

	row_pointers = png_malloc (png_ptr, img.size.y * sizeof (png_byte *));
	int mul = 1;
	if(img.type != 0) {
		mul = 255;
	}
	for (y = 0; y < img.size.y; y++) {
		png_byte *row = png_malloc(png_ptr, sizeof(uint8_t)*img.size.x*pixel_size);
		row_pointers[y] = row;
		for (x = 0; x < img.size.x; x++) {
			unsigned int o = (y*img.size.x+x)*img.size.z;
			for(unsigned int z = 0; z < pixel_size; z++) {
				*row++ = img.image[o+z]*mul;
			}
		}
	}
	
	/* Write the image data to "fp". */

	png_init_io (png_ptr, fp);
	png_set_rows (png_ptr, info_ptr, row_pointers);
	png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	/* The routine has successfully written the file, so we set
		 "status" to a value which indicates success. */

	status = 0;
	
	for (y = 0; y < img.size.y; y++) {
			png_free (png_ptr, row_pointers[y]);
	}
	png_free (png_ptr, row_pointers);
	
png_failure:
png_create_info_struct_failed:
	png_destroy_write_struct (&png_ptr, &info_ptr);
png_create_write_struct_failed:
	fclose (fp);
fopen_failed:
	printf("\n\n\nnoooo!!! image failed!!!\n\n\n");
}

void save_My_png_list(My_png_list images) {
	char file[20] = "";
	for(int i = 0; i < images.len; i++) {
		sprintf(file, "img%d.png", i);
		save_image(images.list[i], file);
	}
}
