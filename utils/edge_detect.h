#ifndef EDGE_DETECT_H
#define EDGE_DETECT_H

extern pval sobel_kernel_array_h[9];
extern pval sobel_kernel_array_v[9];
extern My_png sobel_kernel_h;
extern My_png sobel_kernel_v;
//extern My_png* sobel_kernels[2];

void sobel_accumulator_func(pval* accumulators, pval* accumulator, int depth, int amount);

extern pval ridge_edge_kernel_array_1[9];
extern pval ridge_edge_kernel_array_2[9];
extern My_png ridge_edge_kernel_1;
extern My_png ridge_edge_kernel_2;

//use convolute_img for ridge_edge

extern pval prewitt_kernel_array_h[9];
extern pval prewitt_kernel_array_v[9];
extern My_png prewitt_kernel_h;
extern My_png prewitt_kernel_v;
//extern My_png* prewitt_kernels[2];

//void (*prewitt_accumulator_func)(pval*, pval*, int, int);

extern pval kirsch_kernel_array_1[9];
extern pval kirsch_kernel_array_2[9];
extern pval kirsch_kernel_array_3[9];
extern pval kirsch_kernel_array_4[9];
extern pval kirsch_kernel_array_5[9];
extern pval kirsch_kernel_array_6[9];
extern pval kirsch_kernel_array_7[9];
extern pval kirsch_kernel_array_8[9];
extern My_png kirsch_kernel_1;
extern My_png kirsch_kernel_2;
extern My_png kirsch_kernel_3;
extern My_png kirsch_kernel_4;
extern My_png kirsch_kernel_5;
extern My_png kirsch_kernel_6;
extern My_png kirsch_kernel_7;
extern My_png kirsch_kernel_8;
//extern My_png* kirsch_kernels[8];

void kirsch_accumulator_func(pval* accumulators, pval* accumulator, int depth, int amount);
void kirsch_color_accumulator_func(pval* accumulators, pval* accumulator, int depth, int amount);

extern pval roberts_cross_kernel_array_1[4];
extern pval roberts_cross_kernel_array_2[4];
extern My_png roberts_cross_kernel_1;
extern My_png roberts_cross_kernel_2;
//extern My_png* roberts_cross_kernels[2];

//void (*roberts_cross_accumulator_func)(pval*, pval*, int, int);

//void initialize_variables();

My_png edge_detect_sobel(My_png img); //--FLAG--

My_png edge_detect_ridge_edge(My_png img); //--FLAG--

My_png edge_detect_prewitt(My_png img); //--FLAG--

My_png edge_detect_kirsch(My_png img); //--FLAG--

My_png edge_detect_kirsch_color(My_png img); //--FLAG--

#endif
