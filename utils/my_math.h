#ifndef MY_MATH_H
#define MY_MATH_H

extern double euler;// = 2.71828182845904523536;

extern double pi;// = 3.14159265358979323846;

extern double tau;// = 6.28318530717958647692;

extern double deg60;// = 1.04719755119659774615;

double gaussian_function_2d(double x, double y, double sigma);

xy eval_projection(xyz vertex, xyz campos, xyz theta, xyz campinhole);

xyz scale_xyz(float* v, float s);

#endif
