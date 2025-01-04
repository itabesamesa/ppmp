#ifndef SELECT_H
#define SELECT_H

My_png selection_mask_between(My_png img, color min, color max); //--FLAG--

My_png selection_mask_distance(My_png img, color aim, pval distance); //--FLAG--

My_png selection_mask(My_png img, pval ((*condition_func)(pval))); //--FLAG--no

My_png selection_mask_rgb(My_png img, pval ((*condition_func)(pval*, int))); //--FLAG--no

My_png selection_mask_recursive_distance(My_png img, color aim, pval distance, xy_int start); //--FLAG--

My_png selection_mask_raytrace(My_png img, color error, xy_int center, unsigned int rays); //--FLAG--

#endif
