#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "misc.h"
#include "edge_detect.h"

pval sobel_kernel_array_h[9] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
pval sobel_kernel_array_v[9] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
My_png sobel_kernel_h = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	sobel_kernel_array_h
};
My_png sobel_kernel_v = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	sobel_kernel_array_v
};

/*My_png* sobel_kernel[2] = {
	&sobel_kernel_h,
	&sobel_kernel_v
};*/

void sobel_accumulator_func(pval* accumulators, pval* accumulator, int depth, int amount) {
	for(int z = 0; z < depth; z++) {
		accumulator[z] = sqrt((accumulators[z])*(accumulators[z])+(accumulators[depth+z])*(accumulators[depth+z]));
	}
}

pval ridge_edge_kernel_array_1[9] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
pval ridge_edge_kernel_array_2[9] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
My_png ridge_edge_kernel_1 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	ridge_edge_kernel_array_1
};
My_png ridge_edge_kernel_2 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	ridge_edge_kernel_array_2
};

pval prewitt_kernel_array_h[9] = {1, 1, 1, 0, 0, 0, -1, -1, -1};
pval prewitt_kernel_array_v[9] = {1, 0, -1, 1, 0, -1, 1, 0, -1};
My_png prewitt_kernel_h = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	prewitt_kernel_array_h
};
My_png prewitt_kernel_v = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	prewitt_kernel_array_v
};

/*My_png* prewitt_kernels[2] = {
	&prewitt_kernel_h,
	&prewitt_kernel_v
};*/

//void (*prewitt_accumulator_func)(pval*, pval*, int, int) = sobel_accumulator_func;

pval kirsch_kernel_array_1[9] = {5, 5, 5, -3, 0, -3, -3, -3, -3};
pval kirsch_kernel_array_2[9] = {-3, 5, 5, -3, 0, 5, -3, -3, -3};
pval kirsch_kernel_array_3[9] = {-3, -3, 5, -3, 0, 5, -3, -3, 5};
pval kirsch_kernel_array_4[9] = {-3, -3, -3, -3, 0, 5, -3, 5, 5};
pval kirsch_kernel_array_5[9] = {-3, -3, -3, -3, 0, -3, 5, 5, 5};
pval kirsch_kernel_array_6[9] = {-3, -3, -3, 5, 0, -3, 5, 5, -3};
pval kirsch_kernel_array_7[9] = {5, -3, -3, 5, 0, -3, 5, -3, -3};
pval kirsch_kernel_array_8[9] = {5, 5, -3, -3, 0, -3, 5, -3, -3};
My_png kirsch_kernel_1 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	kirsch_kernel_array_1
};
My_png kirsch_kernel_2 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	kirsch_kernel_array_2
};
My_png kirsch_kernel_3 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	kirsch_kernel_array_3
};
My_png kirsch_kernel_4 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	kirsch_kernel_array_4
};
My_png kirsch_kernel_5 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	kirsch_kernel_array_5
};
My_png kirsch_kernel_6 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	kirsch_kernel_array_6
};
My_png kirsch_kernel_7 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	kirsch_kernel_array_7
};
My_png kirsch_kernel_8 = {
	{3, 3, 1},
	{0, 0},
	2,
	0,
	0,
	kirsch_kernel_array_8
};

/*My_png* kirsch_kernels[8] = {
	&kirsch_kernel_1,
	&kirsch_kernel_2,
	&kirsch_kernel_3,
	&kirsch_kernel_4,
	&kirsch_kernel_5,
	&kirsch_kernel_6,
	&kirsch_kernel_7,
	&kirsch_kernel_8
};*/

void kirsch_accumulator_func(pval* accumulators, pval* accumulator, int depth, int amount) {
	int max_value_index = 0;
	pval max_value_bw = bw(accumulators, depth);
	for(int i = 1; i < amount; i++) {
		pval value = bw(&accumulators[depth*i], depth);
		if(max_value_bw < value) {
			max_value_index = i;
			max_value_bw = value;
		}
	}
	for(int z = 0; z < depth; z++) {
		accumulator[z] = max_value_bw;//accumulators[max_value_index*depth+z];
	}
}

void kirsch_color_accumulator_func(pval* accumulators, pval* accumulator, int depth, int amount) {
int max_value_index = 0;
	pval max_value_bw = bw(accumulators, depth);
	for(int i = 1; i < amount; i++) {
		pval value = bw(&accumulators[depth*i], depth);
		if(max_value_bw < value) {
			max_value_index = i;
			max_value_bw = value;
		}
	}
	for(int z = 0; z < depth; z++) {
		accumulator[z] = accumulators[max_value_index*depth+z];
	}
}
pval roberts_cross_kernel_array_1[4] = {1, 0, 0, -1};
pval roberts_cross_kernel_array_2[4] = {0, 1, -1, 0};
My_png roberts_cross_kernel_1 = {
	{2, 2, 1},
	{0, 0},
	2,
	0,
	0,
	roberts_cross_kernel_array_1
};
My_png roberts_cross_kernel_2 = {
	{2, 2, 1},
	{0, 0},
	2,
	0,
	0,
	roberts_cross_kernel_array_2
};

/*My_png* roberts_cross_kernels[2] = {
	&roberts_cross_kernel_1,
	&roberts_cross_kernel_2
};*/

//void (*roberts_cross_accumulator_func)(pval*, pval*, int, int) = sobel_accumulator_func;

/*void initialize_variables() {
	sobel_kernels[0] = &sobel_kernel_h;
	sobel_kernels[1] = &sobel_kernel_v;
	prewitt_kernels[0] = &prewitt_kernel_h;
	prewitt_kernels[1] = &prewitt_kernel_v;
	kirsch_kernels[0] = &kirsch_kernel_1;
	kirsch_kernels[1] = &kirsch_kernel_2;
	kirsch_kernels[2] = &kirsch_kernel_3;
	kirsch_kernels[3] = &kirsch_kernel_4;
	kirsch_kernels[4] = &kirsch_kernel_5;
	kirsch_kernels[5] = &kirsch_kernel_6;
	kirsch_kernels[6] = &kirsch_kernel_7;
	kirsch_kernels[7] = &kirsch_kernel_8;
	roberts_cross_kernels[0] = &roberts_cross_kernel_1;
	roberts_cross_kernels[1] = &roberts_cross_kernel_2;
}*/

My_png edge_detect_sobel(My_png img) {
	My_png *sobel_kernels[2];
	sobel_kernels[0] = &sobel_kernel_h;
	sobel_kernels[1] = &sobel_kernel_v;
	return convolute_img_multiple_kernels(sobel_kernels, 2, img, sobel_accumulator_func);
}

My_png edge_detect_ridge_edge(My_png img) {
	My_png *roberts_cross_kernels[2];
	roberts_cross_kernels[0] = &roberts_cross_kernel_1;
	roberts_cross_kernels[1] = &roberts_cross_kernel_2;
	return convolute_img_multiple_kernels(roberts_cross_kernels, 2, img, sobel_accumulator_func);
}

My_png edge_detect_prewitt(My_png img) {
	My_png *prewitt_kernels[2];
	prewitt_kernels[0] = &prewitt_kernel_h;
	prewitt_kernels[1] = &prewitt_kernel_v;
	return convolute_img_multiple_kernels(prewitt_kernels, 2, img, sobel_accumulator_func);
}

My_png edge_detect_kirsch(My_png img) {
	My_png *kirsch_kernels[8];
	kirsch_kernels[0] = &kirsch_kernel_1;
	kirsch_kernels[1] = &kirsch_kernel_2;
	kirsch_kernels[2] = &kirsch_kernel_3;
	kirsch_kernels[3] = &kirsch_kernel_4;
	kirsch_kernels[4] = &kirsch_kernel_5;
	kirsch_kernels[5] = &kirsch_kernel_6;
	kirsch_kernels[6] = &kirsch_kernel_7;
	kirsch_kernels[7] = &kirsch_kernel_8;
	return convolute_img_multiple_kernels(kirsch_kernels, 8, img, kirsch_accumulator_func);
}

My_png edge_detect_kirsch_color(My_png img) {
	My_png *kirsch_kernels[8];
	kirsch_kernels[0] = &kirsch_kernel_1;
	kirsch_kernels[1] = &kirsch_kernel_2;
	kirsch_kernels[2] = &kirsch_kernel_3;
	kirsch_kernels[3] = &kirsch_kernel_4;
	kirsch_kernels[4] = &kirsch_kernel_5;
	kirsch_kernels[5] = &kirsch_kernel_6;
	kirsch_kernels[6] = &kirsch_kernel_7;
	kirsch_kernels[7] = &kirsch_kernel_8;
	return convolute_img_multiple_kernels(kirsch_kernels, 8, img, kirsch_color_accumulator_func);
}
