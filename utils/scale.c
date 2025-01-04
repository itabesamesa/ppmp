#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "misc.h"
#include "blur.h"

My_png box_sample(My_png img, float scale) {
	if(scale == 1) {
		return img;
	}
	xyz_int size = img.size;
	size.x *= scale;
	size.y *= scale;
	My_png scaled;
	copy_my_png(&scaled, img);
	scaled.size = size;
	pval* scaleded = malloc(size.x*size.y*size.z*sizeof(pval));
	pval* average = calloc(sizeof(pval), size.z);
	int o = 0;
	if(scale < 1) {
		float sample = 1/scale;
		int sam = ceil(sample);
		My_png kernel = generate_box_sample_blur_kernel(sam, sam);
		for(int y = 0; y < size.y; y++) {
			for(int x = 0; x < size.x; x++) {
				kernel.offset.x = x*sample;
				kernel.offset.y = y*sample;
				//pval* average = calloc(sizeof(pval), size.z);
				accumulate_convolute_kernel(kernel, img, average);
				for(int z = 0; z < size.z; z++) {
					scaleded[o+z] = average[z];
				}
				o += size.z;
			}
		}
	} else {
		for(int y = 0; y < size.y; y++) {
			int yo = floor(y/scale)*img.size.x;
			for(int x = 0; x < size.x; x++) {
				int oimg = (yo+floor(x/scale))*img.size.z;
				for(int z = 0; z < size.z; z++) {
					scaleded[o+z] = img.image[oimg+z];
				}
				o += size.z;
			}
		}
	}
	scaled.image = scaleded;
	return scaled;
}

/*double vec_len(double y1, double x1, double y2, double x2) {
	double x = x2-x1;
	double y = y2-y1;
	return sqrt((x*x)+(y*y));
}*/

My_png bilinear_interpolation(My_png img, float scale){
	if(scale <= 1) {
		return box_sample(img, scale);
	}
	xyz_int size = img.size;
	size.x *= scale;
	size.y *= scale;
	My_png scaled;
	copy_my_png(&scaled, img);
	scaled.size = size;
	pval* scaleded = calloc(sizeof(pval), size.x*size.y*size.z);
	int o = 0;
	/*pval weights[4];
	for(float y = 0; y < size.y; y++) {
		float ys = y/scale;
		int yof = floor(ys);
		int yoc = clip(ceil(ys), 0, img.size.y);
		int yl[2] = {yof, yoc};
		float dify2 = yoc-yof;
		//weights[0] = (yoc-ys)/dify;
		//weights[1] = (ys-yof)/dify;
		float dify = yof-yoc;
		float yfy = yof-ys;
		float yyc = ys-yoc;
		for(float x = 0; x < size.x; x++) {
			float xs = x/scale;
			int xof = floor(xs);
			int xoc = clip(ceil(xs), 0, img.size.x);
			int xl[2] = {xof, xoc};
			float difx = xoc-xof;
			//weights[2] = (xoc-xs)/difx;
			//weights[3] = (xs-xof)/difx;
			//double diag = vec_len(yof, xof, yoc, xoc);
			float xcx = xoc-xs;
			float xxf = xs-xof;
			weights[0] = (xcx*yfy)/(difx*dify);//(pval)clip(diag/(vec_len(ys, xs, yof, xof)), 0, 1);
			if(isnan(weights[0])) {
				weights[0] = (yoc-ys)/dify2;
				if(isnan(weights[0])) {
					weights[0] = 1;
					weights[1] = 0;
					weights[2] = 0;
					weights[3] = 0;
				} else {
					weights[1] = (ys-yof)/dify2;
					weights[2] = (xoc-xs)/difx;
					weights[3] = (xs-xof)/difx;
				}
			} else {
				weights[1] = (xcx*yyc)/(difx*dify);//(pval)clip(diag/(vec_len(ys, xs, yof, xoc)), 0, 1);
				weights[2] = (xxf*yfy)/(difx*dify);//(pval)clip(diag/(vec_len(ys, xs, yoc, xoc)), 0, 1);
				weights[3] = (xxf*yyc)/(difx*dify);//(pval)clip(diag/(vec_len(ys, xs, yoc, xof)), 0, 1);
			}
			for(int i = 0; i < 4; i++) {
				int yi = i>>1;
				int xi = i%2;
				int oimg = ((yl[yi]*img.size.x)+xl[xi])*img.size.z;
				printf("%.3f ", weights[i]);
				//printf("m%d %dm", yi, xi);
				weights[i] = isnan(weights[i])?0:weights[i];
				for(int z = 0; z < size.z; z++) {
					scaleded[o+z] += (pval)(img.image[oimg+z]*weights[i]);
				}
			}
			o += size.z;
		}
	}*/
	xyz_int selectionsize = {2, 2, size.z};
	for(int y = 0; y < size.y; y++) {
		for(int x = 0; x < size.x; x++) {
			xy cord = {(float)x/scale, (float)y/scale};
			if(cord.x == floor(cord.x) && cord.y == floor(cord.y)) {
				for(int z = 0; z < size.z; z++) {
					scaleded[o+z] = img.image[(int)((cord.y*img.size.x+cord.x)*img.size.z+z)];
				}
			} else {
				xy cf = {cord.x-floor(cord.x), cord.y-floor(cord.y)};
				xy cc = {cord.x-ceil(cord.x), cord.y-ceil(cord.y)};
				float weights[4] = {
					sqrt(cf.x*cf.x+cf.y*cf.y),
					sqrt(cc.x*cc.x+cf.y*cf.y),
					sqrt(cf.x*cf.x+cc.y*cc.y),
					sqrt(cc.x*cc.x+cc.y*cc.y)
				};
				/*float dif = (cc.x-cf.x)*(cc.y-cc.x);//does very funky stuff
				float weights[4] = {
					(cf.x*cf.y)/dif,
					(cc.x*cf.y)/dif,
					(cf.x*cc.y)/dif,
					(cc.x*cc.y)/dif
				};*/
				float total = weights[0]+weights[1]+weights[2]+weights[3];
				xy selectionstart = {floor(cord.x), floor(cord.y)};
				My_png boxything = select_subarray_no_offset(selectionstart, selectionsize, img);
				for(int z = 0; z < size.z; z++) {
					scaleded[o+z] =
						boxything.image[z]*(weights[0]/total)+
						boxything.image[boxything.size.z+z]*(weights[1]/total)+
						boxything.image[boxything.size.z*2+z]*(weights[2]/total)+
						boxything.image[boxything.size.z*3+z]*(weights[3]/total);
				}
			}
			o += size.z;
		}
	}
	scaled.image = scaleded;
	return scaled;
}
