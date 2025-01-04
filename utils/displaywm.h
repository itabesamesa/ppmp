#ifndef DISPLAY_H
#define DISPLAY_H

void draw_window(Display* display, Window w, GC gc, window win);

void draw_all(Display* display, Window w, GC gc, win_list win, int depth, int index);

void clear_window(Display* display, Window w, window win);

//int no_preprocessor(pval value);

//int mask_preprocessor(pval value);

void crosshair(Display* display, Window w, GC gc, window win);

void nums(Display* display, Window w, GC gc, window win);

void display_img_function(Display* display, Window w, GC gc, My_png img, window win, void ((*processor)(Display*, GC, int, pval*)));

void display_img(Display* display, Window w, GC gc, My_png pic, window win);

void display_all(Display* display, Window w, GC gc, window win, int amount, My_png images, ...);

void display_all_list(Display* display, Window w, GC gc, window win, int amount, My_png* images);

void display_my_png_list(Display* display, Window w, GC gc, window win, My_png_list images);

//void display_stl(Display* display, Window w, GC gc, window win, xyz campos, xyz theta, xyz campinhole, stl geom);

void display_text(Display* display, Window w, GC gc, window win, text t);

void display_error(Display* display, Window w, GC gc, window win, error err);

void display_XImage_from_My_png(Display* display, Window w, GC gc, My_png img, window win, void ((*processor)(XImage*, int, int, int, pval*)));

#endif
