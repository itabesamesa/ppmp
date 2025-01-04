#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "misc.h"
#include "wm.h"
#include "text.h"
#include "parser.h"
#include "displaywm.h"
#include "gui.h"

typedef struct _X11_stuff {
	XEvent event;
	Display* display;
	Window w;
	GC gc;
	XWindowAttributes info;
} X11_stuff;

void* initialize_gui() {
	X11_stuff* g = malloc(sizeof(X11_stuff));
	g->display = XOpenDisplay(NULL);
	g->w = XCreateSimpleWindow(g->display, DefaultRootWindow(g->display), 0, 0, 250, 250, 1, BlackPixel(g->display, 0), WhitePixel(g->display, 0));
	XMapWindow(g->display, g->w);
	XSelectInput(g->display, g->w, ExposureMask | ButtonPressMask | Button1MotionMask | ButtonReleaseMask | Button3Mask | Button4Mask | Button5Mask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | Mod1Mask);
	g->gc = DefaultGC(g->display, 0);
	XGetWindowAttributes(g->display, g->w, &g->info);
	return g;
}

xy_int get_size(void* info) {
	X11_stuff* g = info;
	return (xy_int){g->info.width, g->info.height};
}

void get_next_event(void* info, win_event* ev) {
	X11_stuff* g = info;
	XEvent event;
	XNextEvent(g->display, &event);
	switch(event.type) {
		case Expose:
			ev->type = DRAW;
			break;
		case ConfigureNotify:
			ev->type = RESIZE;
			ev->resize.x = event.xconfigurerequest.width;
			ev->resize.y = event.xconfigurerequest.height;
			break;
		case MotionNotify:
			ev->type = CURSOR;
			XQueryPointer(
				g->display,
				g->w,
				&event.xbutton.root,
				&event.xbutton.subwindow,
				&event.xbutton.x_root,
				&event.xbutton.y_root,
				&event.xbutton.x,
				&event.xbutton.y,
				&event.xbutton.state
			);
			ev->cursor.x = event.xmotion.x;
			ev->cursor.y = event.xmotion.y;
			break;
		case ButtonPress: {
			ev->type = BUTTON;
			switch(event.xbutton.button) {
				case Button1:
					ev->button = LEFT_CLICK;
					break;
				case Button2:
					ev->button = MIDDEL_CLICK;
					break;
				case Button3:
					ev->button = RIGHT_CLICK;
					break;
				case Button4:
					ev->button = SCROLL_UP;
					break;
				case Button5:
					ev->button = SCROLL_DOWN;
					break;
			}
			break;
		}
		case ButtonRelease: {
			ev->type = BUTTON;
			switch(event.xbutton.button) {
				case Button1:
					ev->button = LEFT_CLICK;
					break;
				case Button2:
					ev->button = MIDDEL_CLICK;
					break;
				case Button3:
					ev->button = RIGHT_CLICK;
					break;
				case Button4:
					ev->button = SCROLL_UP;
					break;
				case Button5:
					ev->button = SCROLL_DOWN;
					break;
			}
			ev->button += 32; //1<<5
			break;
		}
		case KeyPress: {
			int keysyms;
			KeySym key = XkbKeycodeToKeysym(
				g->display,
				event.xkey.keycode,
				0,
				(event.xkey.state&ShiftMask?1:0) //caps lock!
			);
			printf("keysym: %d\n", key);
			ev->key.mask &= ~32;
			ev->key.mask &= ~64;
			switch(key) {
				case 65505: //shift
				case 65506:
					ev->key.mask |= 8; //1<<3
					break;
				case 65509:
					ev->key.mask ^= 4;
					break;
				case 65508:
				case 65507: //ctrl
					ev->key.mask |= 16; //1<<4
					break;
				case 65027:
					ev->key.mask |= 1;
					break;
				default:
					ev->key.mask |= 64;
			}
			ev->key.key = key;
			break;
		}
		case KeyRelease: {
			ev->type = KEY;
			int keysyms;
			KeySym key = XkbKeycodeToKeysym(
				g->display,
				event.xkey.keycode,
				0,
				(event.xkey.state&ShiftMask?1:0) ^ ((ev->key.mask >> 2) & 1)
			);
			printf("keysym: %d\n", key);
			char c;
			ev->key.mask |= 32;
			ev->key.mask &= ~64;
			switch(key) {
				case 65505: //shift
				case 65506:
					ev->key.mask &= ~8; //1<<3
					break;
				case 65508:
				case 65507: //ctrl
					ev->key.mask &= ~16; //1<<4
					break;
				case 65027:
					ev->key.mask &= ~1;
					break;
				default:
					ev->key.mask &= ~64;
			}
			ev->key.key = key;
			break;
		}
		default:
			ev->type = NONE;
	}
}

void draw_windows(void* info, win_list win) {
	X11_stuff* g = info;
	draw_all(g->display, g->w, g->gc, win, -1, 0);
}

void draw_pngs(void* info, window win, My_png_list images) {
	X11_stuff* g = info;
	display_my_png_list(g->display, g->w, g->gc, win, images);
	//display_XImage_from_My_png(g->display, g->w, g->gc, win, images.list[0]);
}

void draw_text(void* info, window win, text t) {
	X11_stuff* g = info;
	display_text(g->display, g->w, g->gc, win, t);
}

void draw_error(void* info, window win, error err) {
	X11_stuff* g = info;
	display_error(g->display, g->w, g->gc, win, err);
}

void empty_window(void* info, window win) {
	X11_stuff* g = info;
	clear_window(g->display, g->w, win);
}

int handle_events(void* info) {return 1;}
