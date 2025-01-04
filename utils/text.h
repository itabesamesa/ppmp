#ifndef TEXT_H
#define TEXT_H

typedef struct _text {
	unsigned int len;
	unsigned int lines;
	unsigned int* linelens;
	unsigned int pos;
	unsigned int prevpos;
	unsigned short added;
	unsigned short tabsize;
	unsigned short showcursor;
	xy_int scrolloffset;
	xy_int textspacing;
	xy_int cursor;
	xy_int prevcursor;
	char* string;
} text;

void add_char(text* t, window* w, char c);

void rm_char(text* t, window* w);

void add_tab(text* t, window* w);

void rm_tab(text* t, window* w);

void move_up(text* t, window* w);

void move_down(text* t, window* w);

void move_right(text* t, window* w);

void move_left(text* t, window* w);

void move_start(text* t, window* w);

void move_end(text* t, window* w);

void move_start_text(text* t, window* w);

void move_end_text(text* t, window* w);

void initialize_text(text* t);

text file_to_text(char* file);

void text_to_file(text t);

#endif
