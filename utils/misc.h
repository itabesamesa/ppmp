#ifndef MISC_H
#define MISC_H

typedef struct _xy {
	float x, y;
} xy;

typedef struct _xy_int {
	int x, y;
} xy_int;

typedef struct _xyz {
	float x, y, z;
} xyz;

typedef struct _xyz_int {
	int x, y, z;
} xyz_int;

typedef float pval;

typedef struct _My_png {
	xyz_int size;
	xy offset; //add a z height (RGBA = 4, RGB = 3, ...)
	short type; //0 = img; 1 = mask; 2 = kernel;
	//odd color types n = n-1+alpha
	unsigned short color_type; //0 = rgb; 2 = hsl; 4 = hsv; 6 = v;
	unsigned short display_type;
	pval* image;
} My_png;

typedef struct _My_png_list {
	unsigned int len;
	My_png* list;
} My_png_list;

typedef struct _polygon {
	unsigned int corners;
	xy_int* p;
} polygon;

typedef struct _color {
	unsigned short type;
	pval* c;
} color;

enum {
	RGB,
	RGBA,
	HSL,
	HSLA,
	HSV,
	HSVA,
	V,
	A
};

enum {
	IMAGE = 0,
	MASK = 1,
	KERNEL = 2
};

enum {
	F5 = 65474,
	LF = 65293,
	DEL = 65288,
	TAB = 65289,
	SHIFT_TAB = 65056,
	UP = 65362,
	LEFT = 65361,
	DOWN = 65364,
	RIGHT = 65363,
	POS1 = 65360,
	NUMPAD_POS1 = 65429,
	END = 65367,
	NUMPAD_END = 65436,
	CAPSLOCK = 65509,
	LEFT_CTRL = 65507,
	RIGHT_CTRL = 65508
};

void add2DVector(xy v, xy* v2);

void sub2DVector(xy v1, xy v2, xy* r);

void scale2DVector(float s, xy* v);

float clip(float value, float min, float max); //--FLAG--

pval max(pval* l, int amount);

pval min(pval* l, int amount);

int color_to_depth(unsigned short type);

color color_to_color(unsigned short type, color c);

pval bw(pval* l, int amount);

void initialize_my_png(My_png* img);

void set_my_png(My_png* img, xyz_int size, xy offset, short type, short color_type, short display_type);

void copy_my_png(My_png* img1, My_png img2);

My_png select_subarray(xy start, xyz_int size, My_png img); //--FLAG--

My_png select_subarray_no_offset(xy start, xyz_int size, My_png img); //--FLAG--

void accumulate_convolute_kernel(My_png kernel, My_png img, pval* accumulator);

My_png convolute_img(My_png kernel, My_png img); //--FLAG--

pval* accumulate_convolute_multiple_kernels(My_png** kernels, int amount, My_png img, xy pos);

My_png convolute_img_multiple_kernels(My_png** kernels, int amount, My_png img, void ((*accumulator_func)(pval*, pval*, int, int)));

void plonk(My_png img, int x, int y);

My_png empty_img(int width, int height, int depth); //--FLAG--

My_png empty_colored_img(int width, int height, int depth, pval* color); //--FLAG--
//length of color must be equal to depth!!!

polygon regular_plygon(unsigned int corners, unsigned int radius, double angle); //--FLAG--

My_png move_img(My_png img, xy pos); //--FLAG--

void test_plonk(My_png img);

void print_My_png(My_png img);

#endif
