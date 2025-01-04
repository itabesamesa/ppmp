#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "misc.h"
#include "mask.h"
#include "select.h"

My_png selection_mask_between(My_png img, color min, color max) {
	My_png selected;
	copy_my_png(&selected, img);
	selected.type = 1;
	min = color_to_color(img.color_type, min);
	max = color_to_color(img.color_type, max);
	pval* selectioned = malloc(img.size.x*img.size.y*img.size.z*sizeof(pval));
	int o = 0;
	for(int y = 0; y < img.size.y; y++) {
		//printf("%.3f %.3f %.3f\n", min.c[0], max.c[0], img.image[o]);
		for(int x = 0; x < img.size.x; x++) {
			for(int z = 0; z < img.size.z; z++) {
				if(img.image[o+z] > min.c[z] && img.image[o+z] < max.c[z]) {
					selectioned[o+z] = 1;
				}
			}
			o += img.size.z;
		}
	}
	selected.image = selectioned;
	return selected;
}

pval pixel_distance(My_png img, color aim, pval distance, unsigned int o) {
	pval dist = 0;
	for(int z = 0; z < img.size.z; z++) {
		pval tmpdist = (aim.c[z]-img.image[o+z])/255;
		dist += tmpdist*tmpdist;
	}
	//printf("%.3f %.3f  ", dist/distance, img.image[o+4]);
return 1-(clip(sqrt(dist), 0, distance)/distance);
}

My_png selection_mask_distance(My_png img, color aim, pval distance) {
	My_png selected;
	copy_my_png(&selected, img);
	selected.type = 1;
	selected.size.z = 1;
	selected.color_type = 6;
	aim = color_to_color(img.color_type, aim);
	pval* selectioned = malloc(img.size.x*img.size.y*sizeof(pval));
	int o = 0;
	pval dist;
	//distance *= distance;
	for(int y = 0; y < img.size.y; y++) {
		for(int x = 0; x < img.size.x; x++) {
			/*dist = 0;
			for(int z = 0; z < img.size.z; z++) {
				dist += (aim.c[z]-img.image[o+z])/distance;
			}
			selectioned[y*img.size.x+x] = clip(-dist/distance, 0, 1);*/
			selectioned[y*img.size.x+x] = pixel_distance(img, aim, distance, o);
			o += img.size.z;
		}
	}
	selected.image = selectioned;
	return selected;
}

My_png selection_mask(My_png img, pval ((*condition_func)(pval))) {
	My_png selected;
	copy_my_png(&selected, img);
	selected.type = 1;
	pval* selectioned = malloc(img.size.x*img.size.y*img.size.z*sizeof(pval));
	int o = 0;
	for(int y = 0; y < img.size.y; y++) {
		for(int x = 0; x < img.size.x; x++) {
			for(int z = 0; z < img.size.z; z++) {
				selectioned[o+z] = (*condition_func)(img.image[o+z]);
			}
			o += img.size.z;
		}
	}
	selected.image = selectioned;
	return selected;
}

My_png selection_mask_rgb(My_png img, pval ((*condition_func)(pval*, int))) {
	My_png selected;
	copy_my_png(&selected, img);
	selected.type = 1;
	selected.size.z = 1;
	pval* selectioned = malloc(img.size.x*img.size.y*sizeof(pval));
	int o = 0;
	for(int y = 0; y < img.size.y; y++) {
		for(int x = 0; x < img.size.x; x++) {
			selectioned[y*img.size.x+x] = (*condition_func)(&(img.image[o]), img.size.z);
			o += img.size.z;
		}
	}
	selected.image = selectioned;
	return selected;
}

//  ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
//  │   │   │   │   │   │   │   │   │   │   │   │
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │   │   │   │   │   │   │   │
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │   │   │   │   │   │   │   │
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │   │   │   │   │   │   │   │
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │   │🮍▒🮌│   │   │   │   │   │
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │🮍▒🮌│▐█▌│🮍▒🮌│   │   │   │   │  check surroundings, then remove pixel pos
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │   │🮍▒🮌│   │   │   │   │   │
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │   │   │   │   │   │   │   │
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │   │   │   │   │   │   │   │
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │   │   │   │   │   │   │   │
//  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
//  │   │   │   │   │   │   │   │   │   │   │   │
//  └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

typedef struct _visited {
	unsigned int* v;
	unsigned int l, w, e, m;
} visited;

void remove_visited(visited* v, unsigned int start) {
	for(unsigned int i = start; i < v->l-1; i++) {
		v->v[i] = v->v[i+1];
	}
	v->l--;
}

void add_visited(visited* v, unsigned value) {
	if(v->m == v->l) {
		v->v = realloc(v->v, sizeof(int)*(v->l+1));
		v->m++;
	}
	v->v[v->l] = value;
	v->l++;
}

short has_not_visited(visited v, unsigned int value) {
	for(unsigned int i = 0; i < v.l; i++) if(v.v[i] == value) return 0;
	return 1;
}

void selection_mask_recursive_distance_inner(My_png img, pval* mask, color aim, pval distance, visited* v, unsigned int depth);

void do_visit(My_png img, pval* mask, visited* v, color aim, pval distance, unsigned int value, unsigned int depth) {
	if(has_not_visited(*v, value)) {
		mask[value/img.size.z] = pixel_distance(img, aim, distance, value);
		add_visited(v, value);
		if(mask[value/img.size.z] > 0.5)
			selection_mask_recursive_distance_inner(img, mask, aim, distance, v, depth-1);
	}
}

void selection_mask_recursive_distance_inner(My_png img, pval* mask, color aim, pval distance, visited* v, unsigned int depth) {
	if(!depth) return;
	unsigned int i = v->l-1;
	int cv = v->v[i];
	int i1 = cv-v->w;
	if(i1 >= (cv % img.size.y)) {
		do_visit(img, mask, v, aim, distance, i1, depth);
	}
	int i2 = cv-img.size.z;
	if(i2 % img.size.y) {
		do_visit(img, mask, v, aim, distance, i2, depth);
	}
	int i3 = cv+img.size.z;
	if(cv % img.size.y) {
		do_visit(img, mask, v, aim, distance, i3, depth);
	}
	int i4 = cv+v->w;
	if(i4 >= v->e) {
		do_visit(img, mask, v, aim, distance, i4, depth);
	}
	remove_visited(v, i);
}

My_png selection_mask_recursive_distance(My_png img, color aim, pval distance, xy_int start) {
	visited v;
	v.v = malloc(sizeof(int));
	v.v[0] = (start.y*img.size.x+start.x)*img.size.z;
	v.l = 1;
	v.w = img.size.x*img.size.z;
	v.e = img.size.y*img.size.x*img.size.z;
	v.m = 1;
	My_png selected;
	copy_my_png(&selected, img);
	selected.type = 1;
	selected.size.z = 1;
	selected.color_type = 6;
	pval* selectioned = calloc(img.size.x*img.size.y, sizeof(pval));
	aim = color_to_color(img.color_type, aim);
	selection_mask_recursive_distance_inner(img, selectioned, aim, distance*distance, &v, 100);
	//printf("%d %d\n", v.l, v.m);
	free(v.v);
	selected.image = selectioned;
	return selected;
}

void get_end(My_png img, xy* end, color min, color max, xy vect);

void get_end(My_png img, xy* end, color min, color max, xy vect) {
	if(end->x < 0 || end->x >= img.size.x || end->y < 0 || end->y >= img.size.y) return;
	unsigned int o = (img.size.x*round(end->y)+round(end->x))*img.size.z;
	for(unsigned int z = 0; z < img.size.z; z++)
		if(img.image[o+z] < min.c[z] || img.image[o+z] > max.c[z]) return;
	//printf("| ");
	end->x += vect.x;
	end->y += vect.y;
	get_end(img, end, min, max, vect);
}

My_png selection_mask_raytrace(My_png img, color error, xy_int center, unsigned int rays) {
	polygon p;
	p.corners = rays;
	p.p = malloc(rays*sizeof(xy_int));
	double angle = (M_PI/rays)*2;
	error = color_to_color(img.color_type, error);
	color min;
	min.type = img.color_type;
	min.c = malloc(sizeof(pval)*img.size.z);
	color max;
	max.type = img.color_type;
	max.c = malloc(sizeof(pval)*img.size.z);
	unsigned int o = (img.size.x*center.y+center.x)*img.size.z;
	for(unsigned int z = 0; z < img.size.z; z++) {
		min.c[z] = img.image[o+z]-error.c[z];
		max.c[z] = img.image[o+z]+error.c[z];
	}
	for(unsigned int i = 0; i < rays; i++) {
		xy vect = {cos(angle*i), sin(angle*i)};
		xy end = {center.x+vect.x, center.y+vect.y};
		get_end(img, &end, min, max, vect);
		//printf(" %.3f %.3f\n", end.x, end.y);
		p.p[i] = (xy_int){end.x, end.y};
	}
	return polygon_mask(p, 1);
}
