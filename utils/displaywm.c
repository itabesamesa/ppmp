#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "misc.h"
#include "wm.h"
#include "my_math.h"
#include "text.h"
#include "parser.h"
#include "displaywm.h"
#include "colors.h"

void draw_window(Display* display, Window w, GC gc, window win) {
	XSetForeground(display, gc, RGB_xyz(win.color));
	XDrawRectangle(display, w, gc, win.offset.x, win.offset.y, win.size.x-1, win.size.y-1);
	XSetForeground(display, gc, 0);
}

void draw_all(Display* display, Window w, GC gc, win_list win, int depth, int index) {
	if(depth != 0 && win.list[index].kids != 0) {
		for(int i = 0; i < win.list[index].kids; i++) {
			draw_all(display, w, gc, win, depth-1, win.list[index].children[i]);
		}
	} else {
		draw_window(display, w, gc, win.list[index]);
	}
}

void clear_window(Display* display, Window w, window win) {
	XClearArea(
		display,
		w,
		win.offset.x+win.border,
		win.offset.y+win.border,
		win.size.x-win.border-1,
		win.size.y-win.border-1,
		0
	);
}

//cull images outside of display //split images into blocks
/*
	┌───────────┐  ┌────╥──────┐
	│~~~~~~~~~~~│  │╳╳╳╳║~~~~~~│
	│~~~~~~~~~~~│  ╞════╬══════╡
	│~~~~~~~~~~~│  │~~~~║~~~~~~│
	│~~~~~~~~~~~│->│~~~~║~~~~~~│
	│~~~~~~~~~~~│  │~~~~║~~~~~~│
	│~~~~~~~~~~~│  │~~~~║~~~~~~│
	│~~~~~~~~~~~│  │~~~~║~~~~~~│
	└───────────┘  └────╨──────┘
*/ //use z value in offset for rendering hirachie and order (top to bottom (0-inf))

unsigned long RGB_long(int r, int g, int b) {return (r<<16)+(g<<8)+b;}

xyz_int RGB_to_xyz_int(unsigned long v) {
	return (xyz_int){
		(int)((char)v>>16),
		(int)((char)v>>8),
		(int)((char)v)
	};
}

int no_preprocessor(pval value) {
	return value;
}

int mask_preprocessor(pval value) {
	return (value*255);
}

void RGB_color_processor(Display* display, GC gc, int o, pval* image) {
	XSetForeground(display, gc,
		RGB_long(
			image[o],//*alpha)+((color.red/256)*(1-alpha)),
			image[o+1],//*alpha)+((color.green/256)*(1-alpha)),
			image[o+2]//*alpha)+((color.blue/256)*(1-alpha))
		)
	);
}

void BW_color_processor(Display* display, GC gc, int o, pval* image) {
	XSetForeground(display, gc,
		RGB_long(
			image[o]*255,//*alpha)+((color.red/256)*(1-alpha)),
			image[o+1]*255,//*alpha)+((color.green/256)*(1-alpha)),
			image[o+2]*255//*alpha)+((color.blue/256)*(1-alpha))
		)
	);
}

void V_color_processor(Display* display, GC gc, int o, pval* image) {
	XSetForeground(display, gc,
		RGB_long(
			image[o]*255,//*alpha)+((color.red/256)*(1-alpha)),
			image[o]*255,//*alpha)+((color.green/256)*(1-alpha)),
			image[o]*255//*alpha)+((color.blue/256)*(1-alpha))
		)
	);
}

void crosshair(Display* display, Window w, GC gc, window win) {
	xy_int s = {
		clip(1, win.size.x, win.pan.x),
		clip(1, win.size.y, win.pan.y),
	};
	//printf("pan: %.3f %.3f %.3f %.3f %.3f %.3f\n", win.pan.x+win.offset.x, win.pan.y+win.offset.y, win.pan.x, win.pan.y, win.offset.x, win.offset.y);
	if(win.pan.x > win.offset.x) {
		XDrawLine(display, w, gc, win.pan.x, win.offset.y, win.pan.x, win.offset.y+win.size.y);
		XDrawLine(display, w, gc, win.pan.x-1, win.offset.y, win.pan.x-1, win.offset.y+win.size.y);
	}
	if(win.pan.y < win.offset.y) {
		XDrawLine(display, w, gc, win.offset.x, win.pan.y, win.offset.x+win.size.x, win.pan.y);
		XDrawLine(display, w, gc, win.offset.x, win.pan.y-1, win.offset.x+win.size.x, win.pan.y-1);
	}
}

void nums(Display* display, Window w, GC gc, window win) {
	char num[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};
	int o;
	xy_int s = {
		clip(0, win.size.x, win.pan.x),
		clip(0, win.size.y, win.pan.y),
	};
	for(int i = 0; i < win.size.x/20; i++) {
		sprintf(num, "%d", (int)(i-s.x/20));
		o = (int)(log10(abs((int)(i-s.x/20))))+((i-s.x/20 < 0)?2:1);
		XDrawString(display, w, gc, i*20, win.offset.y+s.y-5, num, o);
	}
	for(int i = 0; i < win.size.y/20; i++) {
		sprintf(num, "%d", (int)(i-s.y/20));
		o = (int)(log10(abs((int)(i-s.y/20))))+((i-s.y/20 < 0)?2:1);
		XDrawString(display, w, gc, win.offset.x+s.x-5*o-5, i*20, num, o);
	}
}

void display_img_function(Display* display, Window w, GC gc, My_png img, window win, void ((*processor)(Display*, GC, int, pval*))) {
	//printf("%.3f %.3f %.3f %d\n", win.pan.x+img.offset.x-win.offset.x, win.pan.x, img.offset.x, win.offset.x);
	/*win.offset.x += win.border;
	win.offset.y += win.border;
	win.size.x -= win.border;
	win.size.y -= win.border;*/
	xy_int origin = {
		win.pan.x+img.offset.x-win.offset.x,
		win.pan.y+img.offset.y-win.offset.y
	};
	xy_int end = {
		clip(clip(origin.x+img.size.x, 0, win.size.x)-origin.x, 0, win.size.x),
		clip(clip(origin.y+img.size.y, 0, win.size.y)-origin.y, 0, win.size.y),
	};
	origin.x = clip(-origin.x, 0, win.size.x);
	origin.y = clip(-origin.y, 0, win.size.y);
	//printf("origin: %d %d %.3f %d %d\n", origin.x, origin.y, win.pan.x+img.offset.x, (win.pan.x+img.offset.x-win.offset.x), (win.pan.y+img.offset.y-win.offset.y));
	int o;
	//XImage *image;
	//XColor color;
	//Colormap default_cmap = DefaultColormap(display, XDefaultScreen(display));
	xy_int screen;
	//image = XGetImage(display, w, screenx, screeny, enx, eny-1, AllPlanes, XYPixmap);
	//pval alpha;
	if (win.zoom <= 1) {
		if (win.zoom >= 0.1) {
			int inc = (1/win.zoom);
			for(int y = origin.y; y < end.y; y += inc) {
				o = (y*img.size.x+origin.x)*img.size.z;
				for(int x = origin.x; x < end.x; x += inc) {
					//if(img.image[o+3] != 0) {
						screen.x = (x+win.pan.x+img.offset.x)*win.zoom;
						screen.y = (y+win.pan.y+img.offset.y)*win.zoom;
						//color.pixel = XGetPixel(image, x, y);
						//XQueryColor(display, default_cmap, &color);
						//alpha = img.image[o+3]/256;
						/*XSetForeground(display, gc,
							RGB_long(
								preprocessor((img.image[o])),//*alpha)+((color.red/256)*(1-alpha)),
								preprocessor((img.image[o+1])),//*alpha)+((color.green/256)*(1-alpha)),
								preprocessor((img.image[o+2]))//*alpha)+((color.blue/256)*(1-alpha))
							)
						);*/
						processor(display, gc, o, img.image);
						//printf("%d %d %d %.3f\n", color.red/256, color.green/256, color.blue/256, alpha);
						XDrawPoint(display, w, gc, screen.x, screen.y);
					//}
					o += img.size.z*inc;
				}
			}
		}
	} else {
		/*for(int y = originy; y < endy; y++) {
			//o = y*img.size.z*img.size.x;
			o = (y*img.size.x+originx)*img.size.z;
			for(int x = originx; x < endx; x++) {
				//o += floor(x)*img.size.z;
				int posx = x*zoom+pan.x+img.offset.x*zoom;
				int posy = y*zoom+pan.y+img.offset.y*zoom;
				XSetForeground(display, gc, RGB_long(preprocessor(img.image[o]), preprocessor(img.image[o+1]), preprocessor(img.image[o+2])));
				XFillRectangle(display, w, gc, posx, posy, ceil(zoom), ceil(zoom));
				if(ceil(zoom) >= 30) {
					XSetForeground(display, gc, RGB_long(preprocessor(255-img.image[o]), preprocessor(255-img.image[o+1]), preprocessor(255-img.image[o+2])));
					char r[4] = "    ";
					sprintf(r, "%d", (int)img.image[o]);
					char g[4] = "    ";
					sprintf(g, "%d", (int)img.image[o+1]);
					char b[4] = "    ";
					sprintf(b, "%d", (int)img.image[o+2]);
					XDrawString(display, w, gc, posx, posy+10, r, 3);
					XDrawString(display, w, gc, posx, posy+20, g, 3);
					XDrawString(display, w, gc, posx, posy+30, b, 3);
				}
				o += img.size.z;
			}
		}*/
	}
	//XFree(image);
	XSetForeground(display, gc, 0);
}

void RGB_color_processor_XImage(XImage* pic, int x, int y, int o, pval* image) {
	XPutPixel(pic, x, y,
		RGB_long(
			image[o],
			image[o+1],
			image[o+2]
		)
	);
}

void RGBA_color_processor_XImage(XImage* pic, int x, int y, int o, pval* image) {
	xyz_int color = RGB_to_xyz_int(XGetPixel(pic, x, y));
	pval alpha = image[o+3]/255;
	XPutPixel(pic, x, y,
		RGB_long(
			(image[o]*alpha)+((color.x/255)*(1-alpha)),
			(image[o+1]*alpha)+((color.y/255)*(1-alpha)),
			(image[o+2]*alpha)+((color.z/255)*(1-alpha))
		)
	);
}

void HSL_color_processor_XImage(XImage* pic, int x, int y, int o, pval* image) {
	RGB_color_processor_XImage(pic, x, y, 0, HSL_to_RGB(&(image[o])));
}

void HSLA_color_processor_XImage(XImage* pic, int x, int y, int o, pval* image) {
	pval* ca = HSL_to_RGB(&(image[o]));
	pval c[4];
	c[0] = ca[0];
	c[1] = ca[1];
	c[2] = ca[2];
	c[3] = image[o+3];
	RGB_color_processor_XImage(pic, x, y, 0, c);
}

void RGB_mask_color_processor_XImage(XImage* pic, int x, int y, int o, pval* image) {
	XPutPixel(pic, x, y,
		RGB_long(
			image[o]*255,//*alpha)+((color.red/256)*(1-alpha)),
			image[o+1]*255,//*alpha)+((color.green/256)*(1-alpha)),
			image[o+2]*255//*alpha)+((color.blue/256)*(1-alpha))
		)
	);
}

void RGBA_mask_color_processor_XImage(XImage* pic, int x, int y, int o, pval* image) {
	xyz_int color = RGB_to_xyz_int(XGetPixel(pic, x, y));
	pval alpha = image[o+3];
	XPutPixel(pic, x, y,
		RGB_long(
			(image[o]*255*alpha)+((color.x/255)*(1-alpha)),
			(image[o+1]*255*alpha)+((color.y/255)*(1-alpha)),
			(image[o+2]*255*alpha)+((color.z/255)*(1-alpha))
		)
	);
}

void V_color_processor_XImage(XImage* pic, int x, int y, int o, pval* image) {
	XPutPixel(pic, x, y,
		RGB_long(
			image[o]*255,//*alpha)+((color.red/256)*(1-alpha)),
			image[o]*255,//*alpha)+((color.green/256)*(1-alpha)),
			image[o]*255//*alpha)+((color.blue/256)*(1-alpha))
		)
	);
}

void A_color_processor_XImage(XImage* pic, int x, int y, int o, pval* image) {
	xyz_int color = RGB_to_xyz_int(XGetPixel(pic, x, y));
	pval alpha = image[o+3];
	XPutPixel(pic, x, y,
		RGB_long(
			((color.x/255)*(1-alpha)),
			((color.y/255)*(1-alpha)),
			((color.z/255)*(1-alpha))
		)
	);
}

void display_XImage_from_My_png(Display* display, Window w, GC gc, My_png img, window win, void ((*processor)(XImage*, int, int, int, pval*))) {
	xy_int origin = {
		win.pan.x+img.offset.x-win.offset.x,
		win.pan.y+img.offset.y-win.offset.y
	};
	xy_int subies = {
		(origin.x < 0)?0:origin.x,
		(origin.y < 0)?0:origin.y
	};
	xy_int end = {
		clip(clip(origin.x+img.size.x, 0, win.size.x)-subies.x, 0, win.size.x),
		clip(clip(origin.y+img.size.y, 0, win.size.y)-subies.y, 0, win.size.y)
	};

	if(end.x <= 0 || end.y <= 0) {
		return;
	}

	xy_int sampleoffset = {
		clip(-origin.x, 0, win.size.x),
		clip(-origin.y, 0, win.size.y)
	};
	origin.x = clip(origin.x, 0, win.size.x);
	origin.y = clip(origin.y, 0, win.size.y);

	XImage* pic = XGetImage(
		display,
		w,
		origin.x+win.offset.x,
		origin.y+win.offset.y,
		end.x,
		end.y,
		AllPlanes,
		ZPixmap
	);

	unsigned int o = 0;
	/*for(unsigned int y = 0; y < sizey; y++) {
		for(unsigned int x = 0; x < sizex; x++) {
			XPutPixel(pic, x, y, RGB_long(img.image[o], img.image[o+1], img.image[o+2]));
			o += img.size.z;
		}
		o += diff;
	}*/
	for(int y = sampleoffset.y; y < sampleoffset.y+end.y; y++) {
		o = (y*img.size.x)*img.size.z;
		for(int x = sampleoffset.x; x < sampleoffset.x+end.x; x++) {
			int screenx = x-sampleoffset.x;//(x+win.pan.x+img.offset.x)-win.offset.x;
			int screeny = y-sampleoffset.y;//(y+win.pan.y+img.offset.y)-win.offset.y;
			//XPutPixel(pic, screenx, screeny, RGB_long(img.image[o], img.image[o+1], img.image[o+2]));
			processor(pic, screenx, screeny, o+x*img.size.z, img.image);
			//o += img.size.z;
		}
	}

	XPutImage(
		display,
		w,
		gc,
		pic,
		0,
		0,
		origin.x+win.offset.x, //img.offset.x,
		origin.y+win.offset.y, //img.offset.y,
		end.x,
		end.y
	);
	/*for(int y = origin.y; y < end.y; y++) {
		o = (y*img.size.x+origin.x)*img.size.z;
		for(int x = origin.x; x < end.x; x++) {
			int screenx = (x+win.pan.x+img.offset.x)-win.offset.x;
			int screeny = (y+win.pan.y+img.offset.y)-win.offset.y;
			XPutPixel(pic, screenx, screeny, RGB_long(255, 42, 255));
			o += img.size.z;
		}
	}
	XPutImage(
		display,
		w,
		gc,
		pic,
		0,
		0,
		0, //img.offset.x,
		0, //img.offset.y,
		sizex,
		sizey
	);*/
	/*XSetForeground(display, gc, RGB_long(0, 255, 0));
	XDrawRectangle(display, w, gc, origin.x+win.offset.x-1, origin.y+win.offset.y-1, end.x+1, end.y+1);
	XDrawRectangle(display, w, gc, origin.x+win.offset.x-2, origin.y+win.offset.y-2, end.x+3, end.y+3);
	XDrawRectangle(display, w, gc, origin.x+win.offset.x-3, origin.y+win.offset.y-3, end.x+5, end.y+5);
	XDrawRectangle(display, w, gc, origin.x+win.offset.x-4, origin.y+win.offset.y-4, end.x+7, end.y+7);*/
	XDestroyImage(pic);
}

void display_img_switch_color_type(Display* display, Window w, GC gc, My_png img, window win) {
	switch(img.color_type) {
		case 0:
			display_XImage_from_My_png(display, w, gc, img, win, RGB_color_processor_XImage);
			break;
		case 1:
			display_XImage_from_My_png(display, w, gc, img, win, RGBA_color_processor_XImage);
			break;
		case 2:
			display_XImage_from_My_png(display, w, gc, img, win, HSL_color_processor_XImage);
			break;
		case 3:
			display_XImage_from_My_png(display, w, gc, img, win, HSLA_color_processor_XImage);
			break;
		default:
			printf("\n\nColor type not defined!!!!! %d\n\n", img.color_type);
	}
}

void display_mask_switch_color_type(Display* display, Window w, GC gc, My_png img, window win) {
	switch(img.color_type) {
		case 0:
			display_XImage_from_My_png(display, w, gc, img, win, RGB_mask_color_processor_XImage);
			break;
		case 1:
			display_XImage_from_My_png(display, w, gc, img, win, RGBA_mask_color_processor_XImage);
			break;
		case 6:
			display_XImage_from_My_png(display, w, gc, img, win, V_color_processor_XImage);
			break;
		case 7:
			display_XImage_from_My_png(display, w, gc, img, win, A_color_processor_XImage);
			break;
		default:
			printf("\n\nColor type not defined!!!!! %d\n\n", img.color_type);
	}
}


void display_img(Display* display, Window w, GC gc, My_png img, window win) {
	//if(img.color_type != 6) {
		switch(img.type) {
			case 0:
				//display_img_function(display, w, gc, img, win, RGB_color_processor);
				//display_XImage_from_My_png(display, w, gc, img, win, RGB_color_processor_XImage);
				display_img_switch_color_type(display, w, gc, img, win);
				break;
			case 1:
			case 2:
				display_mask_switch_color_type(display, w, gc, img, win);
				break;
			default:
				printf("\n\nType not defined!!!!! %d\n\n", img.type);
		}
	/*} else {
		//display_img_function(display, w, gc, img, win, V_color_processor);
		display_XImage_from_My_png(display, w, gc, img, win, V_color_processor_XImage);
	}*/
	/*switch(img.color_type) {
		case 0:
		case 2:
		case 4:
			display_img_function(display, w, gc, img, win, RGB_color_processor);
			break;
		case 1:
		case 3:
		case 5:
			display_img_function(display, w, gc, img, win, RGB_color_processor);
			break;
		case 6:
			display_img_function(display, w, gc, img, win, BW_color_processor);
			break;
		default:
			printf("\n\nColor type not defined!!!!! %d\n\n", img.color_type);
	}*/
}

void display_all(Display* display, Window w, GC gc, window win, int amount, My_png images, ...) {
	va_list list;
	va_start(list, amount-1);
	display_img(display, w, gc, images, win);
	for(int i = 0; i < amount-1; i++) {
		My_png img = va_arg(list, My_png);
		display_img(display, w, gc, img, win);
	}
	va_end(list);
}

void display_all_list(Display* display, Window w, GC gc, window win, int amount, My_png* images) {
	for(int i = 0; i < amount; i++) display_img(display, w, gc, images[i], win);
}

void display_my_png_list(Display* display, Window w, GC gc, window win, My_png_list images) {
	for(int i = 0; i < images.len; i++) display_img(display, w, gc, images.list[i], win);
}

void display_occlusion(Display* display, Window w, GC gc, window win, int amount, My_png* images) {

}

/*void display_stl(Display* display, Window w, GC gc, xy pan, float zoom, xyz campos, xyz theta, xyz campinhole, stl geom) {
	for(int i = 0; i < geom.amount; i++) {
		xy pos1 = eval_projection(scale_xyz((geom.faces[i]).vertex1, zoom), campos, theta, campinhole);
		xy pos2 = eval_projection(scale_xyz((geom.faces[i]).vertex2, zoom), campos, theta, campinhole);
		xy pos3 = eval_projection(scale_xyz((geom.faces[i]).vertex3, zoom), campos, theta, campinhole);
		//printf("%.0f, %.0f, %.0f, %.0f, %.0f, %.0f\n", pos1.x+100, pos1.y+100, pos2.x+100, pos2.y+100, pos3.x+100, pos3.y+100);
		XDrawLine(display, w, gc, pos1.x+100, pos1.y+100, pos2.x+100, pos2.y+100);
		XDrawLine(display, w, gc, pos2.x+100, pos2.y+100, pos3.x+100, pos3.y+100);
		XDrawLine(display, w, gc, pos3.x+100, pos3.y+100, pos1.x+100, pos1.y+100);
	}
}*/

void display_text(Display* display, Window w, GC gc, window win, text t) {
	XSetForeground(display, gc, 0);
	xy_int offset = {
		win.offset.x+win.pan.x+win.border,
		win.offset.y+win.pan.y+win.border
	};
	printf("%d %d\n", t.added, t.showcursor);
	if(t.showcursor) clear_window(display, w, win);
	//if(t.added == 0) {
		//clear_window(display, w, win);
		unsigned int lf = 1;
		unsigned int x = 0;
		int xstart = -win.pan.x/t.textspacing.x;
		//win.pan.x = clip(t.cursor.x-win.size.x, 0, win.size.x);
		int ystart = -win.pan.y/t.textspacing.y-1;
		for(int i = 0; i < t.lines; i++) {
			int xend = clip(t.linelens[i]-1, xstart, xstart+win.size.x/t.textspacing.x)-xstart;
			if(xend > 0 && i > ystart) {
				XDrawString(
					display,
					w,
					gc,
					win.offset.x+win.border,
					lf*t.textspacing.y+offset.y,
					&t.string[x+xstart],
					xend/*
					clip(
						t.linelens[i]-1+offset.x/t.textspacing.x,
						offset.x/t.textspacing.x,
						(win.size.x-win.border-win.border)/t.textspacing.x
					)*/
				);
			}
			x += t.linelens[i];
			lf++;
		}
	/*} else {
		XClearArea(
			display,
			w,
			t.prevcursor.x*t.textspacing.x+offset.x,
			(t.prevcursor.y-1)*t.textspacing.y+offset.y,
			t.textspacing.x,
			t.textspacing.y,
			0
		);
		if(t.string[t.prevpos] != '\n') {
			XDrawString(
				display,
				w,
				gc,
				t.prevcursor.x*t.textspacing.x+offset.x,
				t.prevcursor.y*t.textspacing.y+offset.y,
				&t.string[t.prevpos],
				1
			);
		}
	}*/
	if(t.showcursor) {
		printf("%d %d %d %d\n", t.cursor.x, t.cursor.y, t.prevcursor.x, t.prevcursor.y);
		XSetForeground(display, gc, 16722687);
		XDrawString(
			display,
			w,
			gc,
			t.cursor.x*t.textspacing.x+offset.x,
			t.cursor.y*t.textspacing.y+offset.y,
			"|",
			1
		);
	}
}

void display_error(Display* display, Window w, GC gc, window win, error err) {
	text t;
	initialize_text(&t);
	t.showcursor = 0;
	t.lines = 2;
	switch (err.type) {
		case 0:
			sprintf(t.string, "Succsess!\0");
			t.lines = 1;
			//t.len = 10;
			//t.linelens[0] = 10;
			break;
		case 1:
			t.linelens = malloc(sizeof(int)*2);
			t.linelens[0] = 34+log10(err.linenum);
			t.linelens[1] = 13+strlen(err.variable);
			t.len = t.linelens[0]+t.linelens[1];
			t.string = malloc(t.len);
			sprintf(
				t.string,
				"Variable not defined in line %d!!!\nVariable: \"%s\"\0",
				err.linenum,
				err.variable
			);
			break;
		case 2:
			t.linelens = malloc(sizeof(int)*2);
			t.linelens[0] = 53+log10(err.linenum)+strlen(err.function);
			t.linelens[1] = 13+strlen(err.variable);
			t.len = t.linelens[0]+t.linelens[1];
			t.string = malloc(t.len);
			sprintf(
				t.string,
				"Variable has wrong type for function \"%s\" in line %d!!!\nVariable: \"%s\"\0",
				err.function,
				err.linenum,
				err.variable
			);
			break;
		case 3:
			t.linelens = malloc(sizeof(int)*2);
			t.linelens[0] = 37+log10(err.linenum);
			t.linelens[1] = 13+strlen(err.function);
			t.len = t.linelens[0]+t.linelens[1];
			t.string = malloc(t.len);
			sprintf(
				t.string,
				"Function in line %d does not exist!!!\nFunction: \"%s\"\0",
				err.linenum,
				err.function
			);
			break;
		case 4:
			t.linelens = malloc(sizeof(int)*2);
			t.linelens[0] = 37+log10(err.linenum);
			t.linelens[1] = 13+strlen(err.variable);
			t.len = t.linelens[0]+t.linelens[1];
			t.string = malloc(t.len);
			sprintf(
				t.string,
				"Variable in line %d does not exist!!!\nVariable: \"%s\"\0",
				err.linenum,
				err.variable
			);
			break;
		default:
			t.linelens = malloc(sizeof(int)*2+1);
			t.linelens[0] = 30;
			t.linelens[1] = 14+log10(err.type);
			t.len = t.linelens[0]+t.linelens[1];
			t.string = malloc(t.len);
			sprintf(
				t.string,
				"Oopsies! Error not defined!!!\nError type: %d\0",
				err.type
			);
	}
	printf("--message--\n%s\n--message--\n", t.string);
	win.pan = (xy){0, win.size.y-3*t.textspacing.y};
	display_text(display, w, gc, win, t);
}
