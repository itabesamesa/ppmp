#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "misc.h"
#include "my_math.h"
#include "colors.h"

pval* HSL_to_RGB(pval* hsl) {
	pval c = (1-abs(2*hsl[1]-1))*hsl[2];
	int hprime = hsl[0]/deg60;
	pval x = c*(1-abs((hprime%2)-1));
	pval m = hsl[2]-(c/2);
	c += m;
	x += m;
	c *= 255;
	x *= 255;
	m *= 255;
	switch(hprime) {
		case 0:
			return (pval[3]){c, x, m};
			break;
		case 1:
			return (pval[3]){x, c, m};
			break;
		case 2:
			return (pval[3]){m, c, x};
			break;
		case 3:
			return (pval[3]){m, x, c};
			break;
		case 4:
			return (pval[3]){x, m, c};
			break;
		case 5:
			return (pval[3]){c, m, x};
			break;
		default:
			return (pval[3]){0, 0, 0};
	}
}

pval* RGB_to_HSL(pval* rgb) {
	pval smallrgb[3] = {rgb[0]/255, rgb[1]/255, rgb[2]/255};
	pval xmax = max(smallrgb, 3);
	pval xmin = min(smallrgb, 3);
	pval c = xmax-xmin;
	pval l = (xmax+xmin)/2;
	pval h;
	if(c == 0) {
		h = 0;
	} else if(xmax == smallrgb[0]) {
		h = deg60*(pval)((int)((smallrgb[1]-smallrgb[2])/c)%6);
	} else if(xmax == smallrgb[1]) {
		h = deg60*(pval)((int)((smallrgb[2]-smallrgb[0])/c)+2);
	} else if(xmax == smallrgb[2]) {
		h = deg60*(pval)((int)((smallrgb[0]-smallrgb[1])/c)+4);
	} else {
		h = 0;
	}
	pval s = (l==0)?0:c/(1-abs(2*xmax-c-1));
	return (pval[3]){h, s, l};
}

pval* HSV_to_RGB(pval* hsv) {
	pval r, g, b;
	pval h = hsv[0], s = hsv[1], v = hsv[2];
	int i = h*6;
	pval f = h*6-i;
	pval p = v*(1-s);
	pval q = v*(1-f*s);
	pval t = v*(1-(1-f)*s);
	switch (i % 6) {
		case 0:
			r = v, g = t, b = p;
			break;
		case 1:
			r = q, g = v, b = p;
			break;
		case 2:
			r = p, g = v, b = t;
			break;
		case 3:
			r = p, g = q, b = v;
			break;
		case 4:
			r = t, g = p, b = v;
			break;
		case 5:
			r = v, g = p, b = q;
			break;
	}
	return (pval[3]){r, g, b};
}

pval* RGB_to_HSV(pval* rgb) {}

My_png img_HSL_to_RGB(My_png img) {
	size_t len = img.size.x*img.size.y*img.size.z*sizeof(pval);
	My_png rgb;
	rgb.image = malloc(len);
	memcpy(rgb.image, img.image, len);
	copy_my_png(&rgb, img);
	rgb.color_type = img.color_type-2;
	int o = 0;
	for(int y = 0; y < rgb.size.y; y++) {
		for(int x = 0; x < rgb.size.x; x++) {
			pval* converted = HSL_to_RGB(&(img.image[o]));
			for(int z = 0; z < 3; z++) {
				rgb.image[o+z] = converted[z];
			}
			o += rgb.size.z;
		}
	}
	return rgb;
}

My_png img_RGB_to_HSL(My_png img) {
	print_My_png(img);
	size_t len = img.size.x*img.size.y*img.size.z*sizeof(pval);
	My_png hsl;
	hsl.image = malloc(len);
	memcpy(hsl.image, img.image, len);
	copy_my_png(&hsl, img);
	hsl.color_type = img.color_type+2;
	int o = 0;
	for(int y = 0; y < img.size.y; y++) {
		for(int x = 0; x < img.size.x; x++) {
			pval* converted = RGB_to_HSL(&(img.image[o]));
			for(int z = 0; z < 3; z++) {
				hsl.image[o+z] = converted[z];
			}
			//printf("%d ", o);
			o += hsl.size.z;
		}
	}
	print_My_png(hsl);
	return hsl;
}
