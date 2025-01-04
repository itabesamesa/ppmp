#ifndef GUI_H
#define GUI_H

/*typedef struct _resize_event {
	xy_int prevsize, newsize;
} resize_event;*/

/*typedef struct _cursor_event {
	xy_int prevpos, newpos;
} cursor_event;*/

typedef enum _button_clicks {
	LEFT_CLICK,
	RIGHT_CLICK,
	MIDDEL_CLICK,
	SCROLL_UP,
	SCROLL_DOWN
} button_clicks;

//typedef struct _button_event {
//	unsigned short type; //0 = left click, 1 = middel click, 2 = right click, 3 = scroll up, 4 = scroll down
	// ored with (pressed or released)?0:(1 << 3);
//} button_event;

typedef struct _key_event {
	int key;
	unsigned short mask; //  0b           0           0           0           0           0           0            0
	//                                    (letter)    (pressed)   (ctrl)      (shift)     (caps_lock) (alt)        (alt_gr)
	char buf[128];
} key_event;

typedef enum _win_event_types {
	NONE,
	DRAW,
	RESIZE,
	CURSOR,
	BUTTON,
	KEY
} win_event_types;

typedef struct _win_event {
	unsigned int type; //0 = none, 1 = draw, 2 = resize, 3 = cursor, 4 = mouse button 5 = key
	//resize_event resize;
	xy_int resize;
	//cursor_event cursor;
	xy_int cursor;
	//button_event button;
	unsigned short button;
	key_event key;
} win_event;

void* initialize_gui();

xy_int get_size(void* info);

void get_next_event(void* info, win_event* ev);

void draw_windows(void* info, win_list win);

void draw_pngs(void* info, window win, My_png_list images);

void draw_text(void* info, window win, text t);

void draw_error(void* info, window win, error err);

void empty_window(void* info, window win);

int handle_events(void* info);

#endif
