#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "wm.h"
#include "text.h"

void shift_char_right(text* t) {
	for(int i = t->len-1; i > t->pos; i--) {
		t->string[i] = t->string[i-1];
	}
}

void shift_char_left(text* t) {
	for(int i = t->pos; i < t->len; i++) {
		t->string[i] = t->string[i+1];
	}
}

void shift_int_right(text* t) {
	for(int i = t->lines-1; i > t->cursor.y; i--) {
		t->linelens[i] = t->linelens[i-1];
	}
}

void shift_int_left(text* t) {
	for(int i = t->cursor.y; i < t->lines; i++) {
		t->linelens[i] = t->linelens[i+1];
	}
}

unsigned int line_len(text t) {
	unsigned int lf[2];
	for(int i = t.pos; i < t.len; i++) {
		if(t.string[i] = '\n') {
			lf[1] = i;
			break;
		}
	}
	for(int i = t.pos; i >= 0; i--) {
		if(t.string[i] = '\n') {
			lf[0] = i;
			break;
		}
	}
	return lf[1]-lf[0];
}

void add_char(text* t, window* w, char c) {
	t->added = 1;
	t->prevpos = t->pos;
	t->prevcursor = t->cursor;
	t->len++;
	t->string = realloc(t->string, t->len);
	shift_char_right(t);
	if(c == '\n') {
		t->lines++;
		t->linelens = realloc(t->linelens, t->lines*sizeof(int));
		shift_int_right(t);
		t->linelens[t->cursor.y] = t->linelens[t->cursor.y-1]-t->cursor.x;
		t->linelens[t->cursor.y-1] = t->cursor.x+1;
		t->cursor.y++;
		t->cursor.x = 0;
		w->pan.x = 0;
		w->pan.y -=
			(((w->pan.y+w->size.y)/t->textspacing.y)-t->cursor.y < t->scrolloffset.y)?
			t->textspacing.y:0;
	} else {
		t->cursor.x++;
		t->linelens[t->cursor.y-1]++;
		w->pan.x -=
			(((w->size.x-w->pan.x)/t->textspacing.x)-t->cursor.x < t->scrolloffset.x)?
			t->textspacing.x:0;
	}
	t->string[t->pos] = c;
	t->pos++;
}

void rm_char(text* t, window* w) {
	if(t->len != 1 && t->pos != 0) {
		t->prevpos = t->pos;
		t->prevcursor = t->cursor;
		t->pos--;
		t->len--;
		printf("%c\n", t->string[t->pos]);
		if(t->string[t->pos] == '\n') {
			t->lines--;
			t->cursor.y--;
			t->cursor.x = t->linelens[t->cursor.y-1]-1;
			t->linelens[t->cursor.y-1] += t->linelens[t->cursor.y]-1;
			shift_int_left(t);
			t->linelens = realloc(t->linelens, t->lines*sizeof(int)+1);
			if(w->pan.y < 0)
				w->pan.y +=
					(t->cursor.y+(w->pan.y/t->textspacing.y) < t->scrolloffset.y)?
					t->textspacing.y:0;
		} else {
			t->cursor.x--;
			t->linelens[t->cursor.y-1]--;
		}
		shift_char_left(t);
		t->string = realloc(t->string, t->len);
	}
}

void add_tab(text* t, window* w) {
	unsigned int prev = t->pos;
	int prevc = t->cursor.x;
	t->pos -= t->cursor.x;
	t->cursor.x = 0;
	for(int i = 0; i < t->tabsize; i++)
		add_char(t, w, ' ');
	t->pos = prev+t->tabsize;
	t->cursor.x = prevc+2;
}

void rm_tab(text* t, window* w) {
	unsigned int prev = t->pos;
	int prevc = t->cursor.x;
	t->pos -= t->cursor.x;
	t->cursor.x = 0;
	for(int i = 0; i < t->tabsize; i++)
		if(t->string[t->pos+i] != ' ')
			return;
	for(int i = 0; i < t->tabsize; i++) {
		t->cursor.x++;
		t->pos++;
		rm_char(t, w);
	}
	t->pos = prev-t->tabsize;
	t->cursor.x = prevc-t->tabsize;
}

void move_up(text* t, window* w) {
	if(t->cursor.y != 1) {
		t->prevpos = t->pos;
		t->prevcursor = t->cursor;
		t->cursor.y--;
		t->pos -= (t->cursor.x+t->linelens[t->cursor.y-1]);
		t->cursor.x = clip(t->cursor.x, 0, t->linelens[t->cursor.y-1]-1);
		t->pos += t->cursor.x;
		t->added = 0;
		if(w->pan.y < 0)
			w->pan.y +=
				(t->cursor.y+(w->pan.y/t->textspacing.y) < t->scrolloffset.y)?
				t->textspacing.y:0;
	}
}

void move_down(text* t, window* w) {
	if(t->cursor.y < t->lines) {
		t->prevpos = t->pos;
		t->prevcursor = t->cursor;
		t->pos += (t->linelens[t->cursor.y-1]-t->cursor.x);
		t->cursor.x = clip(t->cursor.x, 0, t->linelens[t->cursor.y]-1);
		t->pos += t->cursor.x;
		t->cursor.y++;
		t->added = 0;
		w->pan.y -=
			(((w->pan.y+w->size.y)/t->textspacing.y)-t->cursor.y < t->scrolloffset.y)?
			t->textspacing.y:0;
	}
}

void move_right(text* t, window* w) {
	if(t->cursor.x != t->linelens[t->cursor.y-1]-1) {
		t->prevpos = t->pos;
		t->prevcursor = t->cursor;
		t->cursor.x++;
		t->pos++;
		t->added = 0;
		w->pan.x -=
			(((w->size.x-w->pan.x)/t->textspacing.x)-t->cursor.x < t->scrolloffset.x)?
			t->textspacing.x:0;
	}
}

void move_left(text* t, window* w) {
	if(t->cursor.x != 0) {
		t->prevpos = t->pos;
		t->prevcursor = t->cursor;
		t->cursor.x--;
		t->pos--;
		t->added = 0;
		w->pan.x +=
			(t->cursor.x+(w->pan.x/t->textspacing.x) < t->scrolloffset.x)?
			t->textspacing.x:0;
	}
}

void move_start(text* t, window* w) {
	if(t->cursor.x != 0) {
		t->prevpos = t->pos;
		t->prevcursor = t->cursor;
		t->pos -= t->cursor.x;
		t->cursor.x = 0;
		t->added = 0;
		w->pan.x = 0;
	}
}

void move_end(text* t, window* w) {
	if(t->cursor.x != t->linelens[t->cursor.y-1]-1) {
		t->prevpos = t->pos;
		t->prevcursor = t->cursor;
		t->pos += (t->linelens[t->cursor.y-1]-t->cursor.x-1);
		t->cursor.x = t->linelens[t->cursor.y-1]-1;
		t->added = 0;
		int o = t->linelens[t->cursor.y-1]-(w->size.x/t->textspacing.x);
		w->pan.x = (o < 0)?0:-o*t->textspacing.x;
	}
}

void move_start_text(text* t, window* w) {
	t->prevpos = t->pos;
	t->prevcursor = t->cursor;
	t->prevpos = t->pos;
	t->cursor.x = 0;
	t->cursor.y = 1;
	t->pos = 0;
	t->added = 0;
	w->pan.x = 0;
	w->pan.y = 0;
}

void move_end_text(text* t, window* w) {
	t->prevpos = t->pos;
	t->prevcursor = t->cursor;
	t->pos = t->len-1;
	t->cursor.x = t->linelens[t->lines-1]-1;
	t->cursor.y = t->lines;
	t->added = 0;
	int o = t->linelens[t->cursor.y-1]-(w->size.x/t->textspacing.x);
	w->pan.x = (o < 0)?0:-o*t->textspacing.x;
	w->pan.y = //t->lines-((w->size.y+w->pan.y)/t->textspacing.y);
		-((((w->pan.y+w->size.y)/t->textspacing.y)-t->cursor.y < t->scrolloffset.y)?
		t->textspacing.y:0)*t->textspacing.y;
	printf("wsaddsaf%.3f %.3f\n", w->pan.x, w->pan.y);
}

void initialize_text(text* t) {
	t->len = 10;
	t->pos = 9;
	t->prevpos = 9;
	t->lines = 1;
	t->string = malloc(t->len);
	t->showcursor = 1;
	t->cursor = (xy_int){9, 1};
	t->prevcursor = (xy_int){9, 1};
	t->added = 0;
	t->tabsize = 2;
	t->scrolloffset = (xy_int){0, 5};
	t->textspacing = (xy_int){6, 10};
	t->linelens = malloc(sizeof(int));
	t->linelens[0] = 10;
	sprintf(t->string, "type here\0");
}

text file_to_text(char* file) {
	text t;
	initialize_text(&t);
	FILE *f = fopen(file, "r");
	char *line;
	size_t len = 0;
	ssize_t read;
	fseek(f, 0, SEEK_END);
	t.len = ftell(f);
	t.string = malloc(t.len);
	fseek(f, 0, 0);
	read = fread(t.string, 1, t.len, f);
	fseek(f, 0, 0);
	t.linelens = malloc(t.len*sizeof(int));
	t.lines = 0;
	while((read = getline(&line, &len, f) != -1)) {
		t.linelens[t.lines] = strlen(line);
		t.lines++;
	}
	return t;
}

void text_to_file(text t) {
	FILE *f = fopen("./out.txt", "w");
	t.string[t.len] = '\0';
	fprintf(f, t.string);
	fclose(f);
}
