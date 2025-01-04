#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "misc.h"
#include "wm.h"

/*enum {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3
};*/

unsigned long RGB_xyz(xyz_int c) {return (c.x<<16)+(c.y<<8)+c.z;}

//void draw_all(win_list *win, int depth, Display *display, Window w, GC gc);

win_list create_base_window(xy_int size) {
	win_list windows;
	windows.free_indecies_len = 0;
	windows.len = 1;
	windows.list = malloc(sizeof(window));

	windows.list[0].self = 0;
	windows.list[0].parent = 0;
	windows.list[0].direction = EAST;
	windows.list[0].name = malloc(6);
	sprintf(windows.list[0].name, "daddy\0", 6);
	windows.list[0].size = size;
	windows.list[0].offset = (xy_int){0, 0};
	windows.list[0].kids = 0;
	windows.list[0].color = (xyz_int){255, 42, 255};
	windows.list[0].border = 1;
	windows.list[0].zoom = 1;
	windows.list[0].pan = (xy){30, 30};
	return windows;
}

void set_window(window* w, float zoom, xy pan) {

}

unsigned int add_window(win_list* w, int parent, char* name, xyz_int color) {
	w->len++;
	w->list = realloc(w->list, sizeof(window)*w->len);
	w->list[parent].kids++;
	if(w->list[parent].kids == 1) {
		w->list[parent].children = malloc(sizeof(int));
	} else {
		w->list[parent].children = realloc(w->list[parent].children, sizeof(int)*w->list[parent].kids);
	}
	unsigned int index;
	if(w->free_indecies_len == 0) {
		index = w->len-1;
	} else {
		w->free_indecies_len--;
		index = w->free_indecies[w->free_indecies_len];
		w->free_indecies = realloc(w->free_indecies, w->free_indecies_len*sizeof(int));
	}
	w->list[parent].children[w->list[parent].kids-1] = index;
	w->list[index].self = index;
	w->list[index].parent = parent;
	w->list[index].direction = w->list[parent].direction;

	int name_len = strlen(name);
	w->list[index].name = malloc(name_len+1);
	memcpy(w->list[index].name, name, name_len);
	w->list[index].name[name_len] = '\0';
	w->list[index].kids = 0;
	w->list[index].color = color;
	w->list[index].border = w->list[parent].border;
	w->list[index].zoom = 1;
	w->list[index].pan = (xy){0, 0};

	xy_int size = w->list[parent].size;
	switch((w->list[parent].direction)%2) {
		case 0:
			size.y /= w->list[parent].kids;
		case 1:
			size.x /= w->list[parent].kids;
	}
	xy_int offset = w->list[parent].offset;
	for(int i = 0; i < w->list[parent].kids; i++) {
		w->list[w->list[parent].children[i]].size = size;
	}
	switch(w->list[parent].direction%2) {
		case 0:
			for(int i = 0; i < w->list[parent].kids; i++) {
				if(w->list[parent].children[i] == w->list[index].self) {break;}
				offset.y += size.y;
			}
		case 1:
			for(int i = 0; i < w->list[parent].kids; i++) {
				if(w->list[parent].children[i] == w->list[index].self) {break;}
				offset.x += size.x;
			}
	}
	w->list[index].offset = offset;
	return index;
}

void shift_left(unsigned int* array, unsigned int len, unsigned int index) {
	len--;
	if(array[len] != index) {
		int empty = 0;
		for(int i = 0; i < len; i++) {
			if(array[i] == index) {empty++;}
			if(empty == 1) {
				array[i] = array[i+1];
			}
		}
	}
	array = realloc(array, len*sizeof(int));
}

void recalc_window_size_inner(win_list* w, xy_int size, xy_int max, int index) {
	xy ratio = {
		((float)w->list[index].size.x)/((float)w->list[w->list[index].parent].size.x),
		((float)w->list[index].size.y)/((float)w->list[w->list[index].parent].size.y)
	};
	xy_int size2 = {size.x*ratio.x, size.y*ratio.y};
	for(int i = 0; i < w->list[index].kids; i++) {
		recalc_window_size_inner(w, size2, max, w->list[index].children[i]);
	}
	w->list[index].offset.x =
		max.x*(((float)w->list[index].offset.x)/((float)w->list[0].size.x));
	w->list[index].offset.y =
		max.y*(((float)w->list[index].offset.y)/((float)w->list[0].size.y));
	w->list[index].size.x = size2.x;
	w->list[index].size.y = size2.y;
}

void recalc_window_size(win_list* w, xy_int size) {
	recalc_window_size_inner(w, size, size, 0);
}

void rm_window(win_list* w, unsigned int index) {
	if(w->list[index].kids != 0) {
		int o = w->list[w->list[index].parent].kids;
		w->list[w->list[index].parent].kids += w->list[index].kids;
		w->list[w->list[index].parent].children = realloc(
			w->list[w->list[index].parent].children,
			w->list[w->list[index].parent].kids-1
		);
		for(int i = 0; i < w->list[index].kids; i++) {
			w->list[w->list[index].parent].children[o+i] = w->list[index].children[i];
		}
	}
	free(w->list[index].name);
	free(w->list[index].children);
	shift_left(
		w->list[w->list[index].parent].children,
		w->list[w->list[index].parent].kids,
		index
	);
	w->list[w->list[index].parent].kids--;
	//free(w->list[index]);
	w->free_indecies_len++;
	if(w->free_indecies_len == 1) {
		w->free_indecies = malloc(sizeof(int));
	} else {
		w->free_indecies = realloc(w->free_indecies, sizeof(int)*w->free_indecies_len);
	}
	w->free_indecies[w->free_indecies_len-1] = index;
	recalc_window_size_inner(
		w,
		w->list[w->list[index].parent].size,
		w->list[w->list[index].parent].size,
		w->list[index].parent
	);
}

void resize_window(win_list* w, xy_int size, unsigned int index) {
	if(w->list[w->list[index].parent].kids > 1) {
		printf("asfsdf\n");
		xy_int new_size = size;
		xy new_ratio;
		xy ratio;
		switch(w->list[w->list[index].parent].direction%2) {
			case 0:
				new_size.x = w->list[index].size.x;
				new_size.y = clip(
					0,
					size.y,
					w->list[w->list[index].parent].size.y
						-((w->list[w->list[index].parent].kids-1)*10)
				);
				new_ratio.x = 1;
				new_ratio.y = w->list[w->list[index].parent].size.y-new_size.y;
				ratio.x = 1;
				ratio.y = w->list[w->list[index].parent].size.y-w->list[index].size.y;
			case 1:
				new_size.x = clip(
					0,
					size.x,
					w->list[w->list[index].parent].size.x
						-((w->list[w->list[index].parent].kids-1)*10)
				);
				new_size.y = w->list[index].size.y;
				new_ratio.x = w->list[w->list[index].parent].size.x-new_size.x;
				new_ratio.y = 1;
				ratio.x = w->list[w->list[index].parent].size.x-w->list[index].size.x;
				ratio.y = 1;
		}
		printf("%d, %d\n", new_size.x, new_size.y);
		printf("%d, %d, %.3f, %.3f\n", w->list[w->list[index].parent].size.x, w->list[w->list[index].parent].size.y, new_ratio.x, new_ratio.y);
		printf("%d, %d, %.3f, %.3f\n", w->list[index].size.x, w->list[index].size.y, ratio.x, ratio.y);
		w->list[index].size = new_size;
		xy_int o = w->list[w->list[index].parent].offset;
		for(int i = 0; i < w->list[w->list[index].parent].kids; i++) {
			unsigned int in = w->list[w->list[index].parent].children[i];
			printf("%d\n", in);
			w->list[in].offset = o;
			switch(w->list[w->list[in].parent].direction%2) {
				case 0:
					o.y += w->list[in].size.y;
				case 1:
					o.x += w->list[in].size.x;
			}
			if(in != index) {
				xy_int sub_size = {
					(int)(new_ratio.x
						*(((float)w->list[in].size.x)
						/ratio.x)),
					(int)(new_ratio.y
						*(((float)w->list[in].size.y)
						/ratio.y))
				};
				printf("%d, %d\n", sub_size.x, sub_size.y);
				w->list[in].size = sub_size;
				o.x += w->list[in].offset.x;
				o.y += w->list[in].offset.y;
				recalc_window_size_inner(
					w,
					sub_size,
					sub_size,
					in
				);
			}
		}
	}
}

void dump_window_tree_inner(win_list w, int indent, int index) {
	printf("%*s\"%s\"\n%*s%u\n%*s%u\n%*s(%d, %d)\n%*s(%d, %d)\n%*s(%d, %d, %d)\n%*s%.3f\n%*s(%.3f, %.3f)\n%*s%d\n%*s[",
		indent, "name:     ", w.list[index].name,
		indent, "self:     ", w.list[index].self,
		indent, "parent:   ", w.list[index].parent,
		indent, "size:     ", w.list[index].size.x, w.list[index].size.y,
		indent, "offset:   ", w.list[index].offset.x, w.list[index].offset.y,
		indent, "color:    ", w.list[index].color.x, w.list[index].color.y, w.list[index].color.z,
		indent, "zoom:     ", w.list[index].zoom,
		indent, "pan:      ", w.list[index].pan.x, w.list[index].pan.y,
		indent, "kids:     ", w.list[index].kids,
		indent, "children: "
	);
	if(w.list[index].kids != 0) {
		for(int i = 0; i < w.list[index].kids-1; i++) {
			printf("%u, ", w.list[index].children[i]);
		}
		printf("%u]\n\n", w.list[index].children[w.list[index].kids-1]);
	} else {
		printf("]\n\n");
	}
	for(int i = 0; i < w.list[index].kids; i++) {
		dump_window_tree_inner(w, indent+4, w.list[index].children[i]);
	}
}

void dump_window_tree(win_list w) {
	printf("len:              %u\nfree idecies len: %u\nfree indecies:    [",
		w.len, w.free_indecies_len
	);
	if(w.free_indecies_len != 0) {
		for(int i = 0; i < w.free_indecies_len-1; i++) {
			printf("%u, ", w.free_indecies[i]);
		}
		printf("%u]\n\n", w.free_indecies[w.free_indecies_len-1]);
	} else {
		printf("]\n\n");
	}
	dump_window_tree_inner(w, 12, 0);
}
