#ifndef BLEND_H
#define BLEND_H

My_png blend_multply(My_png img1, My_png img2); //--FLAG--

My_png blend_screen(My_png img1, My_png img2); //--FLAG--

My_png blend_overlay(My_png img1, My_png img2); //--FLAG--

My_png blend_hard_light(My_png img1, My_png img2); //--FLAG--

My_png blend_soft_light_photoshop(My_png img1, My_png img2); //--FLAG--

My_png blend_soft_light_pegtop(My_png img1, My_png img2); //--FLAG--

My_png blend_soft_light_illusion_hu(My_png img1, My_png img2); //--FLAG--

My_png blend_soft_light_w3c(My_png img1, My_png img2); //--FLAG--

My_png blend_color_dodge(My_png img1, My_png img2); //--FLAG--

My_png blend_linear_dodge(My_png img1, My_png img2); //--FLAG--

My_png blend_color_burn(My_png img1, My_png img2); //--FLAG--

My_png blend_linear_burn(My_png img1, My_png img2); //--FLAG--

My_png blend_vivid_light(My_png img1, My_png img2); //--FLAG--

My_png blend_linear_light(My_png img1, My_png img2); //--FLAG--

My_png blend_divide(My_png img1, My_png img2); //--FLAG--

My_png blend_add(My_png img1, My_png img2); //--FLAG--

My_png blend_subtract(My_png img1, My_png img2); //--FLAG--

My_png blend_difference(My_png img1, My_png img2); //--FLAG--

My_png blend_darken_only(My_png img1, My_png img2); //--FLAG--

My_png blend_lighten_only(My_png img1, My_png img2); //--FLAG--

My_png blend_average(My_png img1, My_png img2); //--FLAG--

#endif
