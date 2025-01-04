#ifndef WM_H
#define WM_H

enum {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3
};

typedef struct _window {
	unsigned int self;
	unsigned int parent;
	unsigned short direction;
	char* name;
	xy_int size;
	xy_int offset;
	unsigned int kids;
	unsigned int* children;
	xyz_int color;
	unsigned short border;
	float zoom;
	xy pan;
} window;

typedef struct _win_list {
	unsigned int len;
	window* list;
	unsigned int free_indecies_len;
	unsigned int* free_indecies;
} win_list;

unsigned long RGB_xyz(xyz_int c);

win_list create_base_window(xy_int size);

unsigned int add_window(win_list* w, int parent, char* name, xyz_int color);

void shift_letf(unsigned int* array, unsigned int len, unsigned int index);

void recalc_window_size(win_list* w, xy_int size);

void rm_window(win_list* w, unsigned int index);

void resize_window(win_list* w, xy_int size, unsigned int index);

void dump_window_tree_inner(win_list w, int indent, int index);

void dump_window_tree(win_list w);

#endif
