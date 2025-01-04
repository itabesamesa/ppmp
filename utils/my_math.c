#include <stdio.h>
#include <math.h>

#include "misc.h"
#include "my_math.h"

double euler = 2.71828182845904523536;

double pi = 3.14159265358979323846;

double tau = 6.28318530717958647692;

double deg60 = 1.04719755119659774615;

//(1/(2*PI*pow(sigma 2)))*pow(misc::euler -((pow(x 2)+pow(y 2))/(2*pow(sigma 2))))
double gaussian_function_2d(double x, double y, double sigma) {
	double sigmasqr = sigma*sigma;
	double thing = euler-((x*x+y*y)/(2*sigmasqr));
	return ((1/(2*pi*sigmasqr))*thing*thing);
}

xy eval_projection(xyz vertex, xyz campos, xyz theta, xyz campinhole) { //a c theta e
	xyz a = {vertex.x-campos.x, vertex.y-campos.y, vertex.z-campos.z};
	xyz ca = {cos(theta.x), cos(theta.y), cos(theta.z)};
	xyz sa = {sin(theta.x), sin(theta.y), sin(theta.z)};
	float t1 = (ca.y*vertex.z+sa.y*(sa.z*vertex.y+ca.z*vertex.x));
	float t2 = (ca.z*vertex.y-sa.z*vertex.x);
	xyz da = {
		ca.y*(sa.z*vertex.y+ca.z*vertex.x)-sa.y*vertex.z,
		sa.x*t1+ca.z*t2,
		ca.x*t1-sa.x*t2
	};
	xy b = {(campinhole.z*da.x)/(da.z+campinhole.x), (campinhole.z*da.y)/(da.z+campinhole.y)};
	return b;
}

xyz scale_xyz(float* v, float s) {
	return (xyz){v[0]*s, v[1]*s, v[2]*s};
}
