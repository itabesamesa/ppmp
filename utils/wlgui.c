#define _POSIX_C_SOURCE 200112L
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <stdlib.h>
#include <cairo/cairo.h>
#include <pthread.h>
#include "xdg-shell-client-protocol.h"

#include "misc.h"
#include "wm.h"
#include "text.h"
#include "parser.h"
#include "gui.h"

typedef struct _event_buffer {
	win_event* events;
	unsigned int len;
} event_buffer;

event_buffer* evbuff;

void shift_events(event_buffer* ev) {
	for(unsigned int i = 0; i < evbuff->len-1; i++)
		evbuff->events[i] = evbuff->events[i+1];
}

void add_event(event_buffer* ev, unsigned int type) {
	evbuff->len++;
	evbuff->events = realloc(evbuff->events, evbuff->len*sizeof(win_event));
	evbuff->events[evbuff->len-1].type = type;
	//fprintf(stderr, "evbufflen: %d\n", evbuff->len);
}

win_event get_event(event_buffer* ev) {
	win_event event;
	if(evbuff->len) {
		event.type = evbuff->events[0].type;
		event.resize = evbuff->events[0].resize;
		event.cursor = evbuff->events[0].cursor;
		event.button = evbuff->events[0].button;
		event.key = evbuff->events[0].key;
		shift_events(ev);
		evbuff->len--;
		fprintf(stderr, "got event %d\n\n\n", evbuff->events[0].key.mask);
	} else {
		event.type = NONE;
	}
	return event;
}

/* Shared memory support code */
static void
randname(char *buf)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for (int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}

static int
create_shm_file(void)
{
	int retries = 100;
	do {
		char name[] = "/wl_shm-XXXXXX";
		randname(name + sizeof(name) - 7);
		--retries;
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			shm_unlink(name);
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);
	return -1;
}

static int
allocate_shm_file(size_t size)
{
	int fd = create_shm_file();
	if (fd < 0)
		return -1;
	int ret;
	do {
		ret = ftruncate(fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

enum pointer_event_mask {
	POINTER_EVENT_ENTER = 1 << 0,
	POINTER_EVENT_LEAVE = 1 << 1,
	POINTER_EVENT_MOTION = 1 << 2,
	POINTER_EVENT_BUTTON = 1 << 3,
	POINTER_EVENT_AXIS = 1 << 4,
	POINTER_EVENT_AXIS_SOURCE = 1 << 5,
	POINTER_EVENT_AXIS_STOP = 1 << 6,
	POINTER_EVENT_AXIS_DISCRETE = 1 << 7,
};

struct pointer_event {
	uint32_t event_mask;
	wl_fixed_t surface_x, surface_y;
	uint32_t button, state;
	uint32_t time;
	uint32_t serial;
	struct {
		bool valid;
		wl_fixed_t value;
		int32_t discrete;
	} axes[2];
	uint32_t axis_source;
};

enum touch_event_mask {
	TOUCH_EVENT_DOWN = 1 << 0,
	TOUCH_EVENT_UP = 1 << 1,
	TOUCH_EVENT_MOTION = 1 << 2,
	TOUCH_EVENT_CANCEL = 1 << 3,
	TOUCH_EVENT_SHAPE = 1 << 4,
	TOUCH_EVENT_ORIENTATION = 1 << 5,
};

struct touch_point {
	bool valid;
	int32_t id;
	uint32_t event_mask;
	wl_fixed_t surface_x, surface_y;
	wl_fixed_t major, minor;
	wl_fixed_t orientation;
};

struct touch_event {
	uint32_t event_mask;
	uint32_t time;
	uint32_t serial;
	struct touch_point points[10];
};

/* Wayland code */
typedef struct _client_state {
	/* Globals */
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct wl_shm *wl_shm;
	struct wl_compositor *wl_compositor;
	struct xdg_wm_base *xdg_wm_base;
	struct wl_seat *wl_seat;
	/* Objects */
	struct wl_surface *wl_surface;
	struct xdg_surface *xdg_surface;
	struct wl_keyboard *wl_keyboard;
	struct wl_pointer *wl_pointer;
	struct wl_touch *wl_touch;
	struct xdg_toplevel *xdg_toplevel;
	struct xkb_state *xkb_state;
	struct xkb_context *xkb_context;
	struct xkb_keymap *xkb_keymap;
	struct touch_event touch_event;
	/* State */
	float offset;
	uint32_t last_frame;
	int width, height;
	struct pointer_event pointer_event;
	bool closed;
	/* my stuff */
	event_buffer* event_buffer;
	//uint32_t* data;
	uint32_t color;
	uint32_t bg_color;
	int draw_mask;
	win_list win_list;
	window win_pngs, win_t, win_err, win_empty;
	My_png_list images;
	text t;
	error err;
	/* cairo */
	cairo_surface_t* cairo_surface_text;
	cairo_surface_t* cairo_surface_error;
} client_state;;

char* to_bin_wl(int num) {
	char* str = malloc(8);
	for(unsigned int i = 0; i < 8; i++)
		str[i] = ((num >> (8-1-i)) & 1)+48;
	return str;
}

void get_next_event(void* info, win_event* ev) {
	client_state* state = info;
	win_event e = get_event(evbuff);
	/*if(evbuff->len) {
		ev->type = evbuff->events[0].type;
		if(evbuff->events[0].type == KEY) {
			ev->key.mask &= evbuff->events[0].key.mask;
		} else {
			ev->resize = evbuff->events[0].resize;
			ev->cursor = evbuff->events[0].cursor;
			ev->button = evbuff->events[0].button;
		}
		shift_events(state->event_buffer);
		evbuff->len--;
	} else {
		ev->type = NONE;
	}*/
	unsigned short mask = ev->key.mask;
	/*if(evbuff->events[0].type == KEY) {
		mask |= e.key.mask;
	}*/
	memcpy(ev, &e, sizeof(win_event));
	if(!((e.key.mask >> 5) & 1)) {
		ev->key.mask &= ~(mask & 31);
	} else {
		ev->key.mask |= mask & 31;
	}
}

static void
wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{
	fprintf(stderr, "seat name: %s\n", name);
}

static struct touch_point *
get_touch_point(client_state *client_state, int32_t id)
{
	struct touch_event *touch = &client_state->touch_event;
	const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
	int invalid = -1;
	for (size_t i = 0; i < nmemb; ++i) {
		if (touch->points[i].id == id) {
			return &touch->points[i];
		}
		if (invalid == -1 && !touch->points[i].valid) {
			invalid = i;
		}
	}
	if (invalid == -1) {
		return NULL;
	}
	touch->points[invalid].valid = true;
	touch->points[invalid].id = id;
	return &touch->points[invalid];
}

static void
wl_touch_down(void *data, struct wl_touch *wl_touch, uint32_t serial,
               uint32_t time, struct wl_surface *surface, int32_t id,
               wl_fixed_t x, wl_fixed_t y)
{
	client_state *client_state = data;
	struct touch_point *point = get_touch_point(client_state, id);
	if (point == NULL) {
		return;
	}
	point->event_mask |= TOUCH_EVENT_UP;
	point->surface_x = wl_fixed_to_double(x),
			point->surface_y = wl_fixed_to_double(y);
	client_state->touch_event.time = time;
	client_state->touch_event.serial = serial;
}

static void
wl_touch_up(void *data, struct wl_touch *wl_touch, uint32_t serial,
               uint32_t time, int32_t id)
{
	client_state *client_state = data;
	struct touch_point *point = get_touch_point(client_state, id);
	if (point == NULL) {
		return;
	}
	point->event_mask |= TOUCH_EVENT_UP;
}

static void
wl_touch_motion(void *data, struct wl_touch *wl_touch, uint32_t time,
               int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	client_state *client_state = data;
	struct touch_point *point = get_touch_point(client_state, id);
	if (point == NULL) {
		return;
	}
	point->event_mask |= TOUCH_EVENT_MOTION;
	point->surface_x = x, point->surface_y = y;
	client_state->touch_event.time = time;
}

static void
wl_touch_cancel(void *data, struct wl_touch *wl_touch)
{
	client_state *client_state = data;
	client_state->touch_event.event_mask |= TOUCH_EVENT_CANCEL;
}

static void
wl_touch_shape(void *data, struct wl_touch *wl_touch,
               int32_t id, wl_fixed_t major, wl_fixed_t minor)
{
	client_state *client_state = data;
	struct touch_point *point = get_touch_point(client_state, id);
	if (point == NULL) {
		return;
	}
	point->event_mask |= TOUCH_EVENT_SHAPE;
	point->major = major, point->minor = minor;
}

static void
wl_touch_orientation(void *data, struct wl_touch *wl_touch,
               int32_t id, wl_fixed_t orientation)
{
	client_state *client_state = data;
	struct touch_point *point = get_touch_point(client_state, id);
	if (point == NULL) {
		return;
	}
	point->event_mask |= TOUCH_EVENT_ORIENTATION;
	point->orientation = orientation;
}

static void
wl_touch_frame(void *data, struct wl_touch *wl_touch)
{
	client_state *client_state = data;
	struct touch_event *touch = &client_state->touch_event;
	const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
	fprintf(stderr, "touch event @ %d:\n", touch->time);

	for (size_t i = 0; i < nmemb; ++i) {
		struct touch_point *point = &touch->points[i];
		if (!point->valid) {
			continue;
		}
		fprintf(stderr, "point %d: ", touch->points[i].id);

		if (point->event_mask & TOUCH_EVENT_DOWN) {
			fprintf(stderr, "down %f,%f ",
								wl_fixed_to_double(point->surface_x),
								wl_fixed_to_double(point->surface_y));
		}

		if (point->event_mask & TOUCH_EVENT_UP) {
			fprintf(stderr, "up ");
		}

		if (point->event_mask & TOUCH_EVENT_MOTION) {
			fprintf(stderr, "motion %f,%f ",
								wl_fixed_to_double(point->surface_x),
								wl_fixed_to_double(point->surface_y));
		}

		if (point->event_mask & TOUCH_EVENT_SHAPE) {
			fprintf(stderr, "shape %fx%f ",
								wl_fixed_to_double(point->major),
								wl_fixed_to_double(point->minor));
		}

		if (point->event_mask & TOUCH_EVENT_ORIENTATION) {
			fprintf(stderr, "orientation %f ",
								wl_fixed_to_double(point->orientation));
		}

		point->valid = false;
		fprintf(stderr, "\n");
	}
}

static void
wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t format, int32_t fd, uint32_t size)
{
	client_state *client_state = data;
	assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

	char *map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	assert(map_shm != MAP_FAILED);

	struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
								client_state->xkb_context, map_shm,
								XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(map_shm, size);
	close(fd);

	struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
	xkb_keymap_unref(client_state->xkb_keymap);
	xkb_state_unref(client_state->xkb_state);
	client_state->xkb_keymap = xkb_keymap;
	client_state->xkb_state = xkb_state;
}

static void
wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, struct wl_surface *surface,
               struct wl_array *keys)
{
	client_state *client_state = data;
	fprintf(stderr, "keyboard enter; keys pressed are:\n");
	uint32_t *key;
	wl_array_for_each(key, keys) {
		char buf[128];
		xkb_keysym_t sym = xkb_state_key_get_one_sym(
									client_state->xkb_state, *key + 8);
		xkb_keysym_get_name(sym, buf, sizeof(buf));
		fprintf(stderr, "sym: %-12s (%d), ", buf, sym);
		xkb_state_key_get_utf8(client_state->xkb_state,
									*key + 8, buf, sizeof(buf));
		fprintf(stderr, "utf8: '%s'\n", buf);
	}
}

static void
wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	client_state *client_state = data;
	char buf[128];
	uint32_t keycode = key + 8;
	xkb_keysym_t sym = xkb_state_key_get_one_sym(
								client_state->xkb_state, keycode);
	xkb_keysym_get_name(sym, buf, sizeof(buf));
	const char *action =
				state == WL_KEYBOARD_KEY_STATE_PRESSED ? "press" : "release";
	fprintf(stderr, "key %s: sym: %s (%d), ", action, buf, sym);
	xkb_state_key_get_utf8(client_state->xkb_state, keycode,
								buf, sizeof(buf));
	unsigned int cev = evbuff->len;
	fprintf(stderr, "utf8: '%s' %d sym: %d\n", buf, cev, sym);
	add_event(client_state->event_buffer, KEY);
	//client_state->event_buffer->events[cev].key.mask = 0;
	evbuff->events[cev].key.mask = (state == WL_KEYBOARD_KEY_STATE_PRESSED)?32:0;
	evbuff->events[cev].key.key = sym;
	sprintf(evbuff->events[cev].key.buf, "%s", buf);

	switch(sym) {
		case 65505:
		case 65506: //shift
			fprintf(stderr, "shift\n");
			evbuff->events[cev].key.mask += 8;
			break;
		case 65509: //caps_lock
			fprintf(stderr, "caps lock\n");
			evbuff->events[cev].key.mask += 4;
			break;
		case 65508:
		case 65507: //ctrl
			fprintf(stderr, "ctrl\n");
			evbuff->events[cev].key.mask += 16;
			break;
		case 65027: //ALT_GR
			fprintf(stderr, "alt gr\n");
			evbuff->events[cev].key.mask += 1;
			break;
		case 65513: //ALT_L
			fprintf(stderr, "alt\n");
			evbuff->events[cev].key.mask += 2;
			break;
		default:
			fprintf(stderr, "letter\n");
			//evbuff->events[cev].key.mask += 64;
	}
}

static void
wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, struct wl_surface *surface)
{
	fprintf(stderr, "keyboard leave\n");
}

static void
wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, uint32_t mods_depressed,
               uint32_t mods_latched, uint32_t mods_locked,
               uint32_t group)
{
	client_state *client_state = data;
	xkb_state_update_mask(client_state->xkb_state,
				mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

static void
wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
               int32_t rate, int32_t delay)
{
       /* Left as an exercise for the reader */
}

static void
wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
               uint32_t serial, struct wl_surface *surface,
               wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	client_state *client_state = data;
	client_state->pointer_event.event_mask |= POINTER_EVENT_ENTER;
	client_state->pointer_event.serial = serial;
	client_state->pointer_event.surface_x = surface_x,
				client_state->pointer_event.surface_y = surface_y;
}

static void
wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
               uint32_t serial, struct wl_surface *surface)
{
	client_state *client_state = data;
	client_state->pointer_event.serial = serial;
	client_state->pointer_event.event_mask |= POINTER_EVENT_LEAVE;
}

static void
wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time,
               wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	client_state *client_state = data;
	client_state->pointer_event.event_mask |= POINTER_EVENT_MOTION;
	client_state->pointer_event.time = time;
	client_state->pointer_event.surface_x = surface_x,
				client_state->pointer_event.surface_y = surface_y;
	unsigned int cev = evbuff->len;
	add_event(client_state->event_buffer, CURSOR);
	evbuff->events[cev].cursor.x = wl_fixed_to_int(surface_x);
	evbuff->events[cev].cursor.y = wl_fixed_to_int(surface_y);
}

static void
wl_pointer_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial,
               uint32_t time, uint32_t button, uint32_t state)
{
	client_state *client_state = data;
	client_state->pointer_event.event_mask |= POINTER_EVENT_BUTTON;
	client_state->pointer_event.time = time;
	client_state->pointer_event.serial = serial;
	client_state->pointer_event.button = button,
				client_state->pointer_event.state = state;
}

static void
wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time,
               uint32_t axis, wl_fixed_t value)
{
	client_state *client_state = data;
	client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS;
	client_state->pointer_event.time = time;
	client_state->pointer_event.axes[axis].valid = true;
	client_state->pointer_event.axes[axis].value = value;
}

static void
wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
               uint32_t axis_source)
{
	client_state *client_state = data;
	client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_SOURCE;
	client_state->pointer_event.axis_source = axis_source;
}

static void
wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
               uint32_t time, uint32_t axis)
{
	client_state *client_state = data;
	client_state->pointer_event.time = time;
	client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_STOP;
	client_state->pointer_event.axes[axis].valid = true;
}

static void
wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
               uint32_t axis, int32_t discrete)
{
	client_state *client_state = data;
	client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_DISCRETE;
	client_state->pointer_event.axes[axis].valid = true;
	client_state->pointer_event.axes[axis].discrete = discrete;
}

static void
wl_pointer_frame(void *data, struct wl_pointer *wl_pointer)
{
	client_state *client_state = data;
	struct pointer_event *event = &client_state->pointer_event;
	fprintf(stderr, "pointer frame @ %d: ", event->time);

	if (event->event_mask & POINTER_EVENT_ENTER) {
		fprintf(stderr, "entered %f, %f ",
					wl_fixed_to_double(event->surface_x),
					wl_fixed_to_double(event->surface_y));
	}

	if (event->event_mask & POINTER_EVENT_LEAVE) {
		fprintf(stderr, "leave");
	}

	if (event->event_mask & POINTER_EVENT_MOTION) {
		fprintf(stderr, "motion %f, %f ",
					wl_fixed_to_double(event->surface_x),
					wl_fixed_to_double(event->surface_y));
	}

	if (event->event_mask & POINTER_EVENT_BUTTON) {
		char *state = event->state == WL_POINTER_BUTTON_STATE_RELEASED ?
							"released" : "pressed";
		fprintf(stderr, "button %d %s ", event->button, state);
	}

	uint32_t axis_events = POINTER_EVENT_AXIS
				| POINTER_EVENT_AXIS_SOURCE
				| POINTER_EVENT_AXIS_STOP
				| POINTER_EVENT_AXIS_DISCRETE;
	char *axis_name[2] = {
				[WL_POINTER_AXIS_VERTICAL_SCROLL] = "vertical",
				[WL_POINTER_AXIS_HORIZONTAL_SCROLL] = "horizontal",
	};
	char *axis_source[4] = {
				[WL_POINTER_AXIS_SOURCE_WHEEL] = "wheel",
				[WL_POINTER_AXIS_SOURCE_FINGER] = "finger",
				[WL_POINTER_AXIS_SOURCE_CONTINUOUS] = "continuous",
				[WL_POINTER_AXIS_SOURCE_WHEEL_TILT] = "wheel tilt",
	};
	if (event->event_mask & axis_events) {
		for (size_t i = 0; i < 2; ++i) {
			if (!event->axes[i].valid) {
				continue;
		}
			fprintf(stderr, "%s axis ", axis_name[i]);
			if (event->event_mask & POINTER_EVENT_AXIS) {
				fprintf(stderr, "value %f ", wl_fixed_to_double(
												event->axes[i].value));
			}
			if (event->event_mask & POINTER_EVENT_AXIS_DISCRETE) {
				fprintf(stderr, "discrete %d ",
												event->axes[i].discrete);
			}
			if (event->event_mask & POINTER_EVENT_AXIS_SOURCE) {
				fprintf(stderr, "via %s ",
									axis_source[event->axis_source]);
			}
			if (event->event_mask & POINTER_EVENT_AXIS_STOP) {
				fprintf(stderr, "(stopped) ");
			}
		}
	}

	fprintf(stderr, "\n");
	memset(event, 0, sizeof(*event));
}

static const struct wl_touch_listener wl_touch_listener = {
	.down = wl_touch_down,
	.up = wl_touch_up,
	.motion = wl_touch_motion,
	.frame = wl_touch_frame,
	.cancel = wl_touch_cancel,
	.shape = wl_touch_shape,
	.orientation = wl_touch_orientation,
};

static const struct wl_pointer_listener wl_pointer_listener = {
	.enter = wl_pointer_enter,
	.leave = wl_pointer_leave,
	.motion = wl_pointer_motion,
	.button = wl_pointer_button,
	.axis = wl_pointer_axis,
	.frame = wl_pointer_frame,
	.axis_source = wl_pointer_axis_source,
	.axis_stop = wl_pointer_axis_stop,
	.axis_discrete = wl_pointer_axis_discrete,
};

static const struct wl_keyboard_listener wl_keyboard_listener = {
	.keymap = wl_keyboard_keymap,
	.enter = wl_keyboard_enter,
	.leave = wl_keyboard_leave,
	.key = wl_keyboard_key,
	.modifiers = wl_keyboard_modifiers,
	.repeat_info = wl_keyboard_repeat_info,
};

static void
wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{
	client_state *state = data;
	bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

	if (have_pointer && state->wl_pointer == NULL) {
		state->wl_pointer = wl_seat_get_pointer(state->wl_seat);
		wl_pointer_add_listener(state->wl_pointer,
										&wl_pointer_listener, state);
	} else if (!have_pointer && state->wl_pointer != NULL) {
		wl_pointer_release(state->wl_pointer);
		state->wl_pointer = NULL;
	}

	bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

	if (have_keyboard && state->wl_keyboard == NULL) {
		state->wl_keyboard = wl_seat_get_keyboard(state->wl_seat);
		wl_keyboard_add_listener(state->wl_keyboard,
										&wl_keyboard_listener, state);
	} else if (!have_keyboard && state->wl_keyboard != NULL) {
		wl_keyboard_release(state->wl_keyboard);
		state->wl_keyboard = NULL;
	}

	bool have_touch = capabilities & WL_SEAT_CAPABILITY_TOUCH;

	if (have_touch && state->wl_touch == NULL) {
		state->wl_touch = wl_seat_get_touch(state->wl_seat);
		wl_touch_add_listener(state->wl_touch,
										&wl_touch_listener, state);
	} else if (!have_touch && state->wl_touch != NULL) {
		wl_touch_release(state->wl_touch);
		state->wl_touch = NULL;
	}
}

static const struct wl_seat_listener wl_seat_listener = {
	.capabilities = wl_seat_capabilities,
	.name = wl_seat_name,
};

static void
wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
	/* Sent by the compositor when it's no longer using this buffer */
	wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
	.release = wl_buffer_release,
};



xy_int get_size(void* info) {
	client_state* state = info;
	return (xy_int){state->width, state->height};
}

void draw_horizontal(client_state* state, xy_int pos, int length, uint32_t* data) {
	if(pos.y < 0 || pos.y >= state->height || pos.x >= state->width) return;
	if(pos.x < 0) {
		length += pos.x;
		pos.x = 0;
	}
	if(pos.x+length >= state->width) length = state->width-pos.x;
	for(int i = 0; i < length; i++)
		data[pos.y*state->width+pos.x+i] = state->color;
}

void draw_vertical(client_state* state, xy_int pos, int length, uint32_t* data) {
	if(pos.x < 0 || pos.x >= state->width || pos.y >= state->height) return;
	if(pos.y < 0) {
		length += pos.y;
		pos.y = 0;
	}
	if(pos.y+length >= state->height) length = state->height-pos.y;
	for(int i = 0; i < length; i++)
		data[(pos.y+i)*state->width+pos.x] = state->color;
}

void draw_rectangle(client_state* state, xy_int pos, xy_int size, uint32_t* data) {
	draw_horizontal(state, pos, size.x, data);
	draw_vertical(state, pos, size.y, data);
	draw_horizontal(state, (xy_int){pos.x, pos.y+size.y}, size.x, data);
	draw_vertical(state, (xy_int){pos.x+size.x, pos.y}, size.y, data);
}

void fill_rectangle(client_state* state, xy_int pos, xy_int size, uint32_t* data) {
	if(pos.x < 0) {
		size.x += pos.x;
		pos.x = 0;
	}
	if(pos.y < 0) {
		size.y += pos.y;
		pos.y = 0;
	}
	if(pos.x+size.x >= state->width) size.x = state->width-pos.x;
	if(pos.y+size.y >= state->height) size.y = state->height-pos.y;
	for(int y = pos.y; y < pos.y+size.y; y++)
		for(int x = pos.x; x < pos.x+size.x; x++)
			data[y*state->width+x] = state->color;
}

void draw_point(client_state* state, xy_int pos, uint32_t* data) {
	data[pos.y*state->width+pos.x] = state->color;
}

uint32_t RGB_xyz_wl(xyz_int color) {
	return ((255 << 24)+(color.x << 16)+(color.y << 8)+color.z);
}

uint32_t RGB(int R, int G, int B) {
	return ((255 << 24)+(R << 16)+(G << 8)+B);
}

void set_color(client_state* state, uint32_t color) {
	state->color = color;
}

void set_bg_color(client_state* state, uint32_t color) {
	state->bg_color = color;
}

void draw_window(client_state* state, window win, uint32_t* data) {
	set_color(state, RGB_xyz_wl(win.color));
	draw_rectangle(state, win.offset, win.size, data);
}

void draw_all(client_state* state, win_list win, int depth, int index, uint32_t* data);

void draw_all(client_state* state, win_list win, int depth, int index, uint32_t* data) {
	if(depth != 0 && win.list[index].kids != 0) {
		for(int i = 0; i < win.list[index].kids; i++) {
			draw_all(state, win, depth-1, win.list[index].children[i], data);
		}
	} else {
		draw_window(state, win.list[index], data);
	}
}

void draw_windows_wl(client_state* state, uint32_t* data) {
	win_list win = state->win_list;
	draw_all(state, win, -1, 0, data);
}

void empty_window_wl(client_state* state, uint32_t* data) {
	window win = state->win_empty;
	fill_rectangle(state, win.offset, win.size, data);
}

void RGB_color_processor(client_state* state, int o, pval* image) {
	set_color(state,
		RGB(
			image[o],//*alpha)+((color.red/256)*(1-alpha)),
			image[o+1],//*alpha)+((color.green/256)*(1-alpha)),
			image[o+2]//*alpha)+((color.blue/256)*(1-alpha))
		)
	);
}

void BW_color_processor(client_state* state, int o, pval* image) {
	set_color(state,
		RGB(
			image[o]*255,//*alpha)+((color.red/256)*(1-alpha)),
			image[o+1]*255,//*alpha)+((color.green/256)*(1-alpha)),
			image[o+2]*255//*alpha)+((color.blue/256)*(1-alpha))
		)
	);
}

void V_color_processor(client_state* state, int o, pval* image) {
	set_color(state,
		RGB(
			image[o]*255,//*alpha)+((color.red/256)*(1-alpha)),
			image[o]*255,//*alpha)+((color.green/256)*(1-alpha)),
			image[o]*255//*alpha)+((color.blue/256)*(1-alpha))
		)
	);
}

void display_img_function(client_state* state, My_png img, window win, void ((*processor)(client_state*, int, pval*)), uint32_t* data) {
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
							RGB(
								preprocessor((img.image[o])),//*alpha)+((color.red/256)*(1-alpha)),
								preprocessor((img.image[o+1])),//*alpha)+((color.green/256)*(1-alpha)),
								preprocessor((img.image[o+2]))//*alpha)+((color.blue/256)*(1-alpha))
							)
						);*/
						processor(state, o, img.image);
						//printf("%d %d %d %.3f\n", color.red/256, color.green/256, color.blue/256, alpha);
						draw_point(state, screen, data);
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
				XSetForeground(display, gc, RGB(preprocessor(img.image[o]), preprocessor(img.image[o+1]), preprocessor(img.image[o+2])));
				XFillRectangle(display, w, gc, posx, posy, ceil(zoom), ceil(zoom));
				if(ceil(zoom) >= 30) {
					XSetForeground(display, gc, RGB(preprocessor(255-img.image[o]), preprocessor(255-img.image[o+1]), preprocessor(255-img.image[o+2])));
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
}

void display_img(client_state* state, My_png img, window win, uint32_t* data) {
	if(img.color_type != 6) {
		switch (img.type) {
			case 0:
				display_img_function(state, img, win, RGB_color_processor, data);
				break;
			case 1:
			case 2:
				display_img_function(state, img, win, BW_color_processor, data);
				break;
			default:
				printf("\n\nType not defined!!!!! %d\n\n", img.type);
		}
	} else {
		display_img_function(state, img, win, V_color_processor, data);
	}
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

void draw_pngs_wl(client_state* state, uint32_t* data) {
	window win = state->win_pngs;
	My_png_list images = state->images;
	for(int i = 0; i < images.len; i++) display_img(state, images.list[i], win, data);
}

void cairo_surface_onto_wl_shm(client_state* state, cairo_surface_t* surface, window win, uint32_t* data) {
	unsigned char *image_data = cairo_image_surface_get_data(surface);
	int image_width = cairo_image_surface_get_width(surface);
	int imgadd = 0;
	if(image_width > win.size.x) {
		image_width = win.size.x;
		imgadd = image_width-win.size.x;
	}
	int image_height = cairo_image_surface_get_height(surface);
	image_height = (image_height > win.size.y)?win.size.y:image_height;
	int add = state->width-win.offset.x-image_width;

	uint32_t *image_pixel = (uint32_t*)image_data;
	for(int x = 0; x < image_height; x++) {
		for(int y = 0; y < image_width; y++) {
			if(*image_pixel++) *data = *image_pixel;
			data++;
		}
		image_pixel += imgadd;
		data += add;
	}
}

void draw_text_wl(client_state* state, uint32_t* data) {
	window win = state->win_t;
	text t = state->t;
	t.textspacing.y += 10;
	state->cairo_surface_text = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, win.size.x, (t.textspacing.y*t.lines > win.size.x)?win.size.x:t.textspacing.y*t.lines+t.textspacing.y);
	cairo_t* cr = cairo_create (state->cairo_surface_text);
	cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, t.textspacing.y);
	cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);

	unsigned int x = 0;
	for(int i = 0; i < t.lines; i++) {
		cairo_move_to (cr, 0, i*t.textspacing.y+t.textspacing.y);
		char tmptext[t.linelens[i]+1];// = &t.string[x];
		memcpy(tmptext, &t.string[x], t.linelens[i]);
		tmptext[t.linelens[i]] = '\0';
		cairo_show_text (cr, tmptext);
		x += t.linelens[i];
	}

	cairo_surface_onto_wl_shm(state, state->cairo_surface_text, win, data);

	cairo_destroy(cr);
}

void draw_error_wl(client_state* state, uint32_t* data) {
	window win = state->win_err;
	error err = state->err;
	state->cairo_surface_error = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, win.size.x, (20 > win.size.x)?win.size.x:20); //TODO: here also
	cairo_t* cr = cairo_create (state->cairo_surface_error);
	cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, 20); //TODO: no hard code pls
	cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
	switch (err.type) {
		case 0:
			cairo_move_to (cr, 0, 0);
			cairo_show_text (cr, "Succsess!");
			break;
		case 1: {
			char tmpline1[34+(int)log10(err.linenum)];
			sprintf(tmpline1, "Variable not defined in line %d!!!", err.linenum);
			char tmpline2[13+strlen(err.variable)];
			sprintf(tmpline2, "Variable: \"%s\"", err.variable);
			cairo_move_to (cr, 0, 0);
			cairo_show_text (cr, tmpline1);
			cairo_move_to (cr, 0, 10);
			cairo_show_text (cr, tmpline2);
			break;
		}
		case 2: {
			char tmpline1[53+(int)log10(err.linenum)+strlen(err.function)];
			sprintf(tmpline1, "Variable has wrong type for function \"%s\" in line %d!!!", err.function, err.linenum);
			char tmpline2[13+strlen(err.variable)];
			sprintf(tmpline2, "Variable: \"%s\"", err.variable);
			cairo_move_to (cr, 0, 0);
			cairo_show_text (cr, tmpline1);
			cairo_move_to (cr, 0, 10);
			cairo_show_text (cr, tmpline2);
			break;
		}
		case 3: {
			char tmpline1[37+(int)log10(err.linenum)];
			sprintf(tmpline1, "Function in line %d does not exist!!!", err.linenum);
			char tmpline2[13+strlen(err.function)];
			sprintf(tmpline2, "Function: \"%s\"", err.function);
			cairo_move_to (cr, 0, 0);
			cairo_show_text (cr, tmpline1);
			cairo_move_to (cr, 0, 10);
			cairo_show_text (cr, tmpline2);
			break;
		}
		case 4: {
			char tmpline1[37+(int)log10(err.linenum)];
			sprintf(tmpline1, "Variable in line %d does not exist!!!", err.linenum);
			char tmpline2[13+strlen(err.variable)];
			sprintf(tmpline2, "Variable: \"%s\"", err.variable);
			cairo_move_to (cr, 0, 0);
			cairo_show_text (cr, tmpline1);
			cairo_move_to (cr, 0, 10);
			cairo_show_text (cr, tmpline2);
			break;
		}
		default: {
			char tmpline1[30];
			sprintf(tmpline1, "Oopsies! Error not defined!!!");
			char tmpline2[14+(int)log10(err.type)];
			sprintf(tmpline2, "Error type: %d", err.type);
			cairo_move_to (cr, 0, 0);
			cairo_show_text (cr, tmpline1);
			cairo_move_to (cr, 0, 10);
			cairo_show_text (cr, tmpline2);
		}
	}
	//printf("--message--\n%s\n--message--\n", line); //TODO: i wan error in terminal pls

	cairo_surface_onto_wl_shm(state, state->cairo_surface_error, win, data);

	cairo_destroy(cr);
}

void draw_windows(void* info, win_list win) {
	client_state* state = info;
	state->draw_mask |= 1;
	state->win_list = win;
}

void draw_pngs(void* info, window win, My_png_list images) {
	client_state* state = info;
	state->draw_mask |= 1 << 1;
	state->win_pngs = win;
	state->images = images;
}

void draw_text(void* info, window win, text t) {
	client_state* state = info;
	state->draw_mask |= 1 << 2;
	state->win_t = win;
	state->t = t;
	fprintf(stderr, t.string);
}

void draw_error(void* info, window win, error err) {
	client_state* state = info;
	state->draw_mask |= 1 << 3;
	state->win_err = win;
	state->err = err;
}

void empty_window(void* info, window win) {
	client_state* state = info;
	state->draw_mask |= 1 << 4;
	state->win_empty = win;
}

static struct wl_buffer *
draw_frame(client_state *state)
{
	int width = state->width, height = state->height;
	int stride = width * 4;
	int size = stride * height;

	int fd = allocate_shm_file(size);
	if (fd == -1) {
			return NULL;
	}

	uint32_t *data = mmap(NULL, size,
					PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		close(fd);
		return NULL;
	}

	struct wl_shm_pool *pool = wl_shm_create_pool(state->wl_shm, fd, size);
	struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
					width, height, stride, WL_SHM_FORMAT_XRGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);

	/* Draw checkerboxed background */
	/*int offset = (int)state->offset % 8;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if (((x + offset) + (y + offset) / 8 * 8) % 16 < 8)
				data[y * width + x] = 0xFF666666;
			else
				data[y * width + x] = 0xFFEEEEEE;
		}
	}*/
	for(unsigned int i = 0; i < width*height; i++)
		data[i] = state->bg_color;

	if(state->draw_mask & 1) {
		draw_windows_wl(state, data);
		//state->draw_mask &= ~1;
	}
	if((state->draw_mask >> 1) & 1) {
		draw_pngs_wl(state, data);
		//state->draw_mask &= ~(1 << 1);
	}
	if((state->draw_mask >> 2) & 1) {
		draw_text_wl(state, data);
		//state->draw_mask &= ~(1 << 2);
	}
	if((state->draw_mask >> 3) & 1) {
		draw_error_wl(state, data);
		//state->draw_mask &= ~(1 << 3);
	}
	if((state->draw_mask >> 4) & 1) {
		empty_window_wl(state, data);
		//state->draw_mask &= ~(1 << 4);
	}

	//for(unsigned int i = 0; i < height*width; i++) //TODO: change pls
		//data[i] = state->data[i];

	munmap(data, size);
	wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
	return buffer;
}

static void
xdg_toplevel_configure(void *data,
		struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height,
		struct wl_array *states)
{
	client_state *state = data;
	if (width == 0 || height == 0) {
		/* Compositor is deferring to us */
		return;
	}
	printf("size: %d %d %d %d\n", width, height, state->width, state->height);
	state->width = width;
	state->height = height;
	/*state->data = malloc(width*height*4);
	for(unsigned int i = 0; i < width*height; i++)
		state->data[i] = state->bg_color;
	add_event(state->event_buffer, RESIZE);
	add_event(state->event_buffer, DRAW);*/
}

static void
xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
	client_state *state = data;
	state->closed = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_configure,
	.close = xdg_toplevel_close,
};

static void
xdg_surface_configure(void *data,
        struct xdg_surface *xdg_surface, uint32_t serial)
{
	client_state *state = data;
	xdg_surface_ack_configure(xdg_surface, serial);

	struct wl_buffer *buffer = draw_frame(state);
	wl_surface_attach(state->wl_surface, buffer, 0, 0);
	wl_surface_commit(state->wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure,
};

static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
	xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
	.ping = xdg_wm_base_ping,
};

static const struct wl_callback_listener wl_surface_frame_listener;

static void
wl_surface_frame_done(void *data, struct wl_callback *cb, uint32_t time)
{
	wl_callback_destroy(cb);

	client_state *state = data;
	cb = wl_surface_frame(state->wl_surface);
	wl_callback_add_listener(cb, &wl_surface_frame_listener, state);

	if (state->last_frame != 0) {
		int elapsed = time - state->last_frame;
		state->offset += elapsed / 1000.0 * 24;
	}

	struct wl_buffer *buffer = draw_frame(state);
	wl_surface_attach(state->wl_surface, buffer, 0, 0);
	wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
	wl_surface_commit(state->wl_surface);

	state->last_frame = time;
}

static const struct wl_callback_listener wl_surface_frame_listener = {
	.done = wl_surface_frame_done,
};

static void
registry_global(void *data, struct wl_registry *wl_registry,
        uint32_t name, const char *interface, uint32_t version)
{
	client_state *state = data;
	if (strcmp(interface, wl_shm_interface.name) == 0) {
			state->wl_shm = wl_registry_bind(
							wl_registry, name, &wl_shm_interface, 1);
	} else if (strcmp(interface, wl_compositor_interface.name) == 0) {
			state->wl_compositor = wl_registry_bind(
							wl_registry, name, &wl_compositor_interface, 4);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
			state->xdg_wm_base = wl_registry_bind(
							wl_registry, name, &xdg_wm_base_interface, 1);
			xdg_wm_base_add_listener(state->xdg_wm_base,
							&xdg_wm_base_listener, state);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
			state->wl_seat = wl_registry_bind(
							wl_registry, name, &wl_seat_interface, 7);
			wl_seat_add_listener(state->wl_seat,
							&wl_seat_listener, state);
			}
}

static void
registry_global_remove(void *data,
        struct wl_registry *wl_registry, uint32_t name)
{
	/* This space deliberately left blank */
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove,
};

/*void* create_wl_display(void* info) {
	client_state* state = info;
	fprintf(stderr, "started display\n");
	while(wl_display_dispatch(state->wl_display)) {}
}*/

int handle_events(void* info) {
	client_state* state = info;
	int i = wl_display_dispatch(state->wl_display);
	//printf("%d\n", i);
	return i;
}

void* initialize_gui() {
	client_state* state = malloc(sizeof(client_state));
	state->width = 640;
	state->height = 480;
	state->color = 0xFF000000;
	state->bg_color = 0xFFFFFFFF;
	state->wl_display = wl_display_connect(NULL);
	state->wl_registry = wl_display_get_registry(state->wl_display);
	state->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	state->event_buffer = malloc(sizeof(event_buffer));
	state->event_buffer->events = malloc(sizeof(win_event));
	state->event_buffer->len = 1;
	state->event_buffer->events[0].type = DRAW;
	evbuff = malloc(sizeof(event_buffer));
	evbuff->events = malloc(sizeof(win_event));
	evbuff->len = 1;
	evbuff->events[0].type = DRAW;
	wl_registry_add_listener(state->wl_registry, &wl_registry_listener, state);
	wl_display_roundtrip(state->wl_display);

	state->wl_surface = wl_compositor_create_surface(state->wl_compositor);
	state->xdg_surface = xdg_wm_base_get_xdg_surface(
					state->xdg_wm_base, state->wl_surface);
	xdg_surface_add_listener(state->xdg_surface, &xdg_surface_listener, state);
	state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
	xdg_toplevel_add_listener(state->xdg_toplevel,
			&xdg_toplevel_listener, state);
	xdg_toplevel_set_title(state->xdg_toplevel, "Example client");
	wl_surface_commit(state->wl_surface);

	/*printf("malloc data\n");
	state->data = malloc(state->width*state->height*4);
	for(unsigned int i = 0; i < state->width*state->height; i++)
		state->data[i] = state->bg_color;*/

	struct wl_callback *cb = wl_surface_frame(state->wl_surface);
	wl_callback_add_listener(cb, &wl_surface_frame_listener, state);

	//while(wl_display_dispatch(state->wl_display)) {};
	//pthread_create(&state->lmt, NULL, create_wl_display, (void*)state);

	return state;
}
