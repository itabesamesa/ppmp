#ifndef MASK_H
#define MASK_H

My_png rectangle_mask(xyz_int size, xy_int start, xy_int end); //--FLAG--

My_png circle_mask(xyz_int size, float radius, xy_int pos); //--FLAG--

//My_png regular_polygon_mask(xyz_int size, int corners, float radius);

//My_png polygon_mask(xyz_int size, xy_int* corners, int amount);

My_png apply_mask(My_png img, My_png mask, xyz_int size, int img_depth_offset); //--FLAG--

My_png polygon_mask(polygon poly, int depth); //--FLAG--

#endif
