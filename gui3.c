//#include <X11/Xlib.h>
//#include <X11/XKBlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <png.h>

//my libraries
#include "utils/misc.h"
#include "utils/wm.h"
#include "utils/text.h"
#include "utils/parser.h"
#include "utils/pngtopng.h"
#include "utils/stl/load_stl.h"
#include "utils/my_math.h"
#include "utils/colors.h"
//#include "utils/displaywm.h"
#include "utils/gui.h"
#include "utils/edge_detect.h"
#include "utils/blur.h"
#include "utils/scale.h"
#include "utils/flip.h"
#include "utils/skew.h"
#include "utils/rotate.h"
#include "utils/mask.h"
#include "utils/posterize.h"
#include "utils/noise.h"
#include "utils/select.h"
#include "utils/blend.h"

#define INT_LEN sizeof(int)*2

int intlength(int n) {
	return floor(log10(n))+1;
}

char* to_bin(int num) {
	char* str = malloc(INT_LEN+1);
	for(unsigned int i = 0; i < INT_LEN; i++)
		str[i] = ((num >> (INT_LEN-1-i)) & 1)+48;
	return str;
}

int main(int argc, char** argv) {
	if(argc < 2) {
		printf("No file specified. Exiting\n");
		return 1;
	}
	printf("Using file: %s\n", argv[1]);

	xy_int pointerPos = {0, 0};

	void* info = initialize_gui();

	My_png img = openimg(argv[1]);

	xy_int screen_size = get_size(info);
	int intlw = intlength(screen_size.x);
	int intlh = intlength(screen_size.y);
	char strwidth[intlw];
	char strheight[intlh];
	sprintf(strwidth, "width: %d", screen_size.x);
	sprintf(strheight, "height: %d", screen_size.y);
	intlw += 7;
	intlh += 8;

	win_list windows = create_base_window(screen_size);
	//windows.list[0].direction = NORTH;
	unsigned int k1 = add_window(&windows, 0, "k1", (xyz_int){0, 0, 255});
	unsigned int k2 = add_window(&windows, 0, "k2", (xyz_int){255, 0, 0});
	//unsigned int k21 = add_window(&windows, 2, "k2.1", (xyz_int){0, 0, 255});
	//unsigned int k22 = add_window(&windows, 2, "k2.2", (xyz_int){255, 42, 255});
	//rm_window(&windows, k22);
	//k22 = add_window(&windows, 2, "k2.2", (xyz_int){255, 42, 255});
	//resize_window(&windows, (xy_int){50, 50}, k1);
	dump_window_tree(windows);

	window cwin = windows.list[k2];

	xyz_int size = {100, 100, 4};
	xy offset = {0, 0};
	My_png subselectionthing = select_subarray(offset, size, img);
	//My_png kernel;
	//kernel.offset.x = 0;
	//kernel.offset.y = 0;
	//kernel.size.x = 3;
	//kernel.size.y = 3;
	//kernel.image = malloc(9*sizeof(pval));
	//pval kernelarray[] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
	//pval kernelarray[] = {0.111, 0.111, 0.111, 0.111, 0.111, 0.111, 0.111, 0.111, 0.111};
	//kernel.image = kernelarray;
	//My_png kernel = generate_box_sample_blur_kernel(20, 20);

	My_png *sobel_kernels[2];
	sobel_kernels[0] = &sobel_kernel_h;
	sobel_kernels[1] = &sobel_kernel_v;
	My_png *prewitt_kernels[2];
	prewitt_kernels[0] = &prewitt_kernel_h;
	prewitt_kernels[1] = &prewitt_kernel_v;
	My_png *kirsch_kernels[8];
	kirsch_kernels[0] = &kirsch_kernel_1;
	kirsch_kernels[1] = &kirsch_kernel_2;
	kirsch_kernels[2] = &kirsch_kernel_3;
	kirsch_kernels[3] = &kirsch_kernel_4;
	kirsch_kernels[4] = &kirsch_kernel_5;
	kirsch_kernels[5] = &kirsch_kernel_6;
	kirsch_kernels[6] = &kirsch_kernel_7;
	kirsch_kernels[7] = &kirsch_kernel_8;
	My_png *roberts_cross_kernels[2];
	roberts_cross_kernels[0] = &roberts_cross_kernel_1;
	roberts_cross_kernels[1] = &roberts_cross_kernel_2;

	int amount = sizeof(sobel_kernels)/sizeof(sobel_kernels[0]);
	//printf("%d\n%d\n%d\n\n", amount, sizeof(sobel_kernels), sizeof(sobel_kernels[0]));
	//My_png sharpend = convolute_img(sobel_kernel_v, img);
	//accumulator_func = &sobel_accumulator_func;
	//My_png sharpend = convolute_img_multiple_kernels(kirsch_kernels, amount, subselectionthing, kirsch_accumulator_func);
	double angle = 6.5;
	//My_png sharpend = skew_horizontal(subselectionthing, -tan(angle/2));
	//xy_int possss = {20, 20};
	//xy_int possss2 = {50, 60};
	//My_png sharpend = circle_mask(subselectionthing.size, 50, possss);
	//xyz_int rec_mask_size = subselectionthing.size;
	//rec_mask_size.z = 1;
	//My_png sharpend = rectangle_mask(rec_mask_size, possss, possss2);
	//pval color[4] = {255, 42, 255, 255};
	//My_png sharpend = empty_colored_img(100, 100, 4, color);
	color min;
	min.type = 1;
	min.c = malloc(4*sizeof(pval));
	min.c[0] = 80;
	min.c[1] = 40;
	min.c[2] = 40;
	min.c[3] = 40;
	color max;
	max.type = 1;
	max.c = malloc(4*sizeof(pval));
	max.c[0] = 255;
	max.c[1] = 0;
	max.c[2] = 0;
	max.c[3] = 0;
	//My_png sharpend = selection_mask_between(subselectionthing, min, max);
	My_png sharpend = selection_mask_distance(subselectionthing, min, 100);
	//My_png sharpend = selection_mask_recursive_distance(subselectionthing, min, 1.5, (xy_int){50, 50});
	//My_png sharpend = selection_mask_raytrace(subselectionthing, min, (xy_int){50, 50}, 20);

	//My_png kernel = generate_box_sample_blur_kernel(3, 3);
	//My_png subselection = convolute_img(kernel, subselectionthing);
	//My_png subselection = flip_vertical(subselectionthing);
	//My_png subselection = skew_vertical(subselectionthing, -0.785);
	//My_png subselection = rotate_180(skew_horizontal(subselectionthing, 0.785));
	//My_png subselection = rotate_skew(subselectionthing, 0.785);
	//My_png subselection = skew_vertical(sharpend, sin(angle));
	//My_png pink_square = empty_colored_img(100, 100, 4, color);
	//My_png subselection = rotate_rotation_matrix(subselectionthing, angle);
	polygon p;// = regular_plygon(5, 20, M_PI/5);
	p.corners = 5;
	p.p = malloc(5*sizeof(xy_int));
	p.p[0] = (xy_int){0, 0};
	p.p[1] = (xy_int){100, 0};
	p.p[2] = (xy_int){0, 100};
	p.p[3] = (xy_int){100, 50};
	p.p[4] = (xy_int){75, 75};
	//p.p[0] = (xy_int){0, 0};
	//p.p[1] = (xy_int){20, 10};
	//p.p[2] = (xy_int){30, 30};
	//p.p[3] = (xy_int){30, 40};
	//p.p[4] = (xy_int){10, 20};
	//p.p[0] = (xy_int){-60, 0};
	//p.p[1] = (xy_int){-20, 0};
	//p.p[2] = (xy_int){-7, -38};
	//p.p[3] = (xy_int){-40, -61};
	//p.p[4] = (xy_int){-72, -38};
	My_png subselection = polygon_mask(p, 1);
	//My_png subselection = apply_mask(subselectionthing, sharpend, subselectionthing.size, 0);

	//My_png scaletioned = box_sample(subselectionthing, 0.5);
	//My_png scaletioned = bilinear_interpolation(subselectionthing, 2.5);
	//My_png scaletioned = flip_horizontal(subselectionthing);
	//My_png scaletioned = skew_horizontal(subselectionthing, 0.785);
	//My_png scaletioned = skew_horizontal(subselection, -tan(angle/2));
	//My_png scaletioned = rotate_rotation_matrix(subselectionthing, angle);
	//My_png scaletioned = empty_img(100, 100, 4);
	//test_plonk(scaletioned);
	//My_png scaletioned = div_clip(subselectionthing, 40);
	//My_png scaletioned = worley_noise_2d(img.size, 30, 12345);
	//xyz_int perlin_vector = {100, 100, 1};
	//My_png scaletioned = perlin_noise(perlin_vector, 10, 1);
	//My_png scaletioned = div_clip(subselectionthing, 100);
	//printf("fdhghfdsghfdsghkfdsglkjhdslkgjhfdskjghkjdsh%d %d %f %f\n", subselectionthing.size.x, subselectionthing.size.y, subselectionthing.offset.x, subselectionthing.offset.y);
	//My_png scaletioned = selection_mask_distance(subselectionthing, color, 50);

	My_png rgbtohsl = img_RGB_to_HSL(subselectionthing);
	My_png hsltorgb = img_HSL_to_RGB(rgbtohsl);

	subselectionthing.offset = (xy){-50, -50};
	My_png scaletioned = blend_multply(hsltorgb, subselectionthing);
	print_My_png(scaletioned);

	xyz_int img_size = subselectionthing.size;
	//printf("ghkjdsh%d %d %f %f\n", img_size.x, img_size.y, subselectionthing.offset.x, subselectionthing.offset.y);
	subselectionthing.offset.x = 10;
	subselectionthing.offset.y = 10;
	sharpend.offset.x = 10;
	sharpend.offset.y = 10;
	//My_png scaletioned = apply_mask(subselectionthing, sharpend, img_size, 0);

	sharpend.offset.x = 300;
	sharpend.offset.y = 175;
	subselection.offset.x = 425;
	subselection.offset.y = 175;
	subselectionthing.offset.x = 300;
	subselectionthing.offset.y = 25;
	scaletioned.offset.x = 425;
	scaletioned.offset.y = 25;
	hsltorgb.offset.x = 550;
	hsltorgb.offset.y = 25;
	rgbtohsl.offset.x = 550;
	rgbtohsl.offset.y = 175;

	text script;
	initialize_text(&script);

	My_png_list images;
	images.len = 7;
	images.list = malloc(sizeof(My_png)*7);
	images.list[0] = img;
	images.list[1] = subselection;
	images.list[2] = subselectionthing;
	images.list[3] = scaletioned;
	images.list[4] = sharpend;
	images.list[5] = hsltorgb;
	images.list[6] = rgbtohsl;

	win_event event;
	event.key.mask = 0;

	while(handle_events(info)) {
		get_next_event(info, &event);
		if(event.type) printf("%d\n", event.type);
		switch(event.type) {
			case DRAW:
				draw_windows(info, windows);
				draw_pngs(info, cwin, images);
				draw_text(info, windows.list[k1], script);
				break;
			case RESIZE:
				if(event.resize.x != screen_size.x && event.resize.y != screen_size.y) {
					screen_size.x = event.resize.x;
					screen_size.y = event.resize.y;
					recalc_window_size(&windows, screen_size);
					//dump_window_tree(windows);
					cwin = windows.list[k2];
					empty_window(info, cwin);
					draw_pngs(info, cwin, images);
					draw_windows(info, windows);
					pointerPos = (xy_int){0, 0};
				}
				break;
			case CURSOR:
				xy_int currentPointerPos = event.cursor;
				//sub2DVector(currentPointerPos, pointerPos, &pointerPos);
				//add2DVector(pointerPos, &cwin.pan);

				cwin.pan.x += currentPointerPos.x-pointerPos.x;
				cwin.pan.y += currentPointerPos.y-pointerPos.y;

				pointerPos = currentPointerPos;

				char coordinates[20];
				sprintf(coordinates, "x=%d, y=%d", currentPointerPos.x, currentPointerPos.y);
				int coordslen = 5+intlength(currentPointerPos.x)+intlength(currentPointerPos.y)+1;
				printf("%s", coordinates);
				empty_window(info, cwin);

				draw_pngs(info, cwin, images);
				//crosshair(display, w, gc, cwin);

				//XDrawString(display, w, gc, 10+cwin.offset.x, 10, strwidth, intlw);
				//XDrawString(display, w, gc, 10+cwin.offset.x, 20, strheight, intlh);
				//XDrawString(display, w, gc, 10+cwin.offset.x, 30, coordinates, coordslen);

				//nums(display, w, gc, cwin);
				break;
/*			case ButtonPress: {
				XNextEvent(display, &event);
				if(event.type == ButtonRelease) {break;}
				XQueryPointer(
					display,
					w,
					&event.xbutton.root,
					&event.xbutton.subwindow,
					&event.xbutton.x_root,
					&event.xbutton.y_root,
					&event.xbutton.x,
					&event.xbutton.y,
					&event.xbutton.state
				);

				pointerPos.x = event.xbutton.x;
				pointerPos.y = event.xbutton.y;

				while (event.type == MotionNotify) {
					XQueryPointer(
						display,
						w,
						&event.xbutton.root,
						&event.xbutton.subwindow,
						&event.xbutton.x_root,
						&event.xbutton.y_root,
						&event.xbutton.x,
						&event.xbutton.y,
						&event.xbutton.state
					);
					xy currentPointerPos = {event.xmotion.x, event.xmotion.y};
					sub2DVector(currentPointerPos, pointerPos, &pointerPos);
					add2DVector(pointerPos, &cwin.pan);

					pointerPos = currentPointerPos;

					char coordinates[20];
					sprintf(coordinates, "x=%d, y=%d", event.xmotion.x, event.xmotion.y);
					int coordslen = 5+intlength(event.xbutton.x)+intlength(event.xbutton.y)+1;
					XClearWindow(display, w);

					display_all(display, w, gc, cwin, 1, img, subselection, subselectionthing, scaletioned, sharpend);
					crosshair(display, w, gc, cwin);
					nums(display, w, gc, cwin);

					XDrawString(display, w, gc, 10, 10, strwidth, intlw);
					XDrawString(display, w, gc, 10, 20, strheight, intlh);
					XDrawString(display, w, gc, 10, 30, coordinates, coordslen);
				}
				switch (event.xbutton.button) {
					case Button4:
						cwin.zoom += 0.1;
						break;
					case Button5:
						cwin.zoom -= 0.1;
						break;
					default: break;
				}
				break;
			}*/
			case KEY: {
				printf("key pressed: %d\n", (event.key.mask >> 5) & 1);
				printf("key mask: %s %d test: %s %s key: |%s| |%c|\n", to_bin(event.key.mask), event.key.mask, to_bin(64), to_bin(event.key.mask & 31), event.key.buf, event.key.key);
				if((event.key.mask >> 5) & 1) {
					switch(event.key.key) {
						case 65474: //F5
							printf("interprating...\n");
							error err = interprate(script);
							draw_error(info, cwin, err);
							if(err.type == 0) images = err.images;
							draw_pngs(info, cwin, images);
							break;
						case 65293: //LF
							add_char(&script, &windows.list[k1],  '\n');
							break;
						case 65288: //DEL
							rm_char(&script, &windows.list[k1]);
							break;
						case 65289: //TAB
							printf("add tab\n");
							add_tab(&script, &windows.list[k1]);
							break;
						case 65056: //SHIFT TAB
							printf("rm tab\n");
							rm_tab(&script, &windows.list[k1]);
							break;
						case 65362: //up
							move_up(&script, &windows.list[k1]);
							break;
						case 65361: //left
							move_left(&script, &windows.list[k1]);
							break;
						case 65364: //down
							move_down(&script, &windows.list[k1]);
							break;
						case 65363: //right
							move_right(&script, &windows.list[k1]);
							break;
						case 65360:
						case 65429: //Pos1
							if((event.key.mask >> 4) & 1) {
								move_start_text(&script, &windows.list[k1]);
							} else {
								move_start(&script, &windows.list[k1]);
							}
							break;
						case 65367:
						case 65436: //Ende
							if((event.key.mask >> 4) & 1) {
								move_end_text(&script, &windows.list[k1]);
							} else {
								move_end(&script, &windows.list[k1]);
							}
							break;
						default:
							if((event.key.mask >> 4) & 1) {
								switch (event.key.key) {
									case 99: //c
										empty_window(info, windows.list[0]);
										draw_windows(info, windows);
										draw_pngs(info, cwin, images);
										draw_text(info, windows.list[k1], script);
										break;
									case 101:
										//Pixmap pix = XCreatePixmap(display, w, 100, 100, 1);
										//XWriteBitmapFile(display, "./bitmap_from_gui.bmp", pix, screen_size.x, screen_size.y, -1, -1);
										save_My_png_list(images);
										break;
									case 111: //o
										script = file_to_text("./out.txt");
										break;
									case 113: //q
										return 0;
									case 115: //s
										text_to_file(script);
										break;
								}
							} else {
								if(event.key.mask & 1) {
									switch(event.key.key) {
										case 55: add_char(&script, &windows.list[k1], '{'); break;
										case 56: add_char(&script, &windows.list[k1], '['); break;
										case 57: add_char(&script, &windows.list[k1], ']'); break;
										case 48: add_char(&script, &windows.list[k1], '}'); break;
										case 223: add_char(&script, &windows.list[k1], '\\'); break;
									}
								} else {
									if(event.key.key >= 32 && 126 >= event.key.key) {
										add_char(&script, &windows.list[k1], (char)event.key.key);
									}
								}
							}
					}
					//printf("%c\n", c);
					//XDrawString(display, w, gc, 10, 40, script.string, script.len-1);
					//clear_window(display, w, windows.list[k1]);
					draw_text(info, windows.list[k1], script);
					//printf("%s", script.string);
				} /*else {
					switch(event.key.key) {
					}
				}*/
				break;
			}
			default:
				//printf("%d, %d\n", screen_size.x, screen_size.y);
				//printf("%s\n", script.string);
		}
	}
	return 0;
}
