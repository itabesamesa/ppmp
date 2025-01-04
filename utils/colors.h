#ifndef COLORS_H
#define COLORS_H

pval* HSL_to_RGB(pval* hsl);

pval* RGB_to_HSL(pval* rgb);

pval* HSV_to_RGB(pval* hsv);

pval* RGB_to_HSV(pval* rgb);

My_png img_HSL_to_RGB(My_png img); //--FLAG--

My_png img_RGB_to_HSL(My_png img); //--FLAG--

#endif
