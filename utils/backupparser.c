#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <png.h>

//my libraries
#include "misc.h"
#include "wm.h"
#include "text.h"
#include "parser.h"
#include "functions.h"

#include "pngtopng.h"
#include "stl/load_stl.h"
#include "my_math.h"
#include "colors.h"
#include "displaywm.h"
#include "edge_detect.h"
#include "blur.h"
#include "scale.h"
#include "flip.h"
#include "skew.h"
#include "rotate.h"
#include "mask.h"
#include "posterize.h"
#include "noise.h"
#include "gui.h"

int rmwhitespace(char* line, int start) {
	for(int i = start-1; i > 0; i--)
		if(!(line[i] == ' ' || line[i] == '\n'))
			return i+1;
	return 0;
}

int rmleadingwhitespace(char* line, int end) {
	for(int i = 0; i < end; i++)
		if(!(line[i] == ' ' || line[i] == '\n'))
			return i;
	return end;
}

int getnext(char* line, int end) {
	for(int i = 0; i < end; i++)
		if(line[i] == ' ' || line[i] == '\n')
			return i;
	return end;
}

int getend(char* line, char delim, int end) {
	for(int i = 0; i < end; i++)
		if(line[i] == delim)
			return i;
	return end;
}

xyz_int arraytoxyz_int(char* array) {
	char* endptr;
	xyz_int pos;
	pos.x = strtol(array, &endptr, 10);
	pos.y = strtol(endptr, &endptr, 10);
	pos.z = strtol(endptr, &endptr, 10);
	return pos;
}

xy_int arraytoxy_int(char* array) {
	char* endptr;
	xy_int pos;
	pos.x = strtol(array, &endptr, 10);
	pos.y = strtol(endptr, &endptr, 10);
	return pos;
}

xy arraytoxy(char* array) {
	char* endptr;
	xy pos;
	pos.x = strtof(array, &endptr);
	pos.y = strtof(endptr, &endptr);
	return pos;
}

int checktype(char varval) {
	if(varval == '\"' || varval == '\'') {
		return 1;
	} else if(varval >= '0' && varval <= '9') {
		return 2;
	} else if(varval == '{') {
		return 3;
	}
	return -1;
}

int cmpvariables(var* variables, int varamount, char* cmpvar) {
	for(int i = 0; i < varamount; i++)
		if(strcmp(cmpvar, variables[i].name) == 0)
			return i;
	return -1;
}

error make_error(unsigned int type, char* function, unsigned int linenum, char* variable, unsigned int arraylen) {
	error err;
	err.type = type;
	switch (type) {
		case 0: return err;
		case 5:
			err.arraylen = arraylen;
		case 2:
			err.function = malloc(strlen(function));
			strcpy(err.function, function);
		case 4:
		case 1:
			err.variable = malloc(strlen(variable));
			strcpy(err.variable, variable);
			break;
		case 3:
			err.function = malloc(strlen(function));
			strcpy(err.function, function);
			break;
	}
	err.linenum = linenum;
	return err;
}

/*int make_bitmask(int amount, ...) {
	va_list list;
	va_start(list, amount-1);
	int mask = 0;
	for(int i = 0; i < amount-1; i++) {
		mask += list[i] << i*2;
	}
	va_end(list);
	return mask;
}*/

/*inputs avtypes_to_inputs(int bitmask) {
	inputs in;
	in.haserror = 0;
	int ored1 = ~1;
	bitmask ~= bitmask;
	unsigned short strnum = 0;
	unsigned short numnum = 0;
	unsigned short arraynum = 0;
	for(int i = 1; i < o; i++) {
		if(((bitmask >> (options[i].type+3)) | ored1) == 0) {
			in.haserror = 1;
			in.err = make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
			return in;
		}
		switch(options[i].type) {
			case 1:
				if(strnum) {
					in.string = realloc(in.string, sizeof(char *)*(strnum+1))
				} else {
					in.string = malloc(sizeof(char *)*(strnum+1))
				}
				in.string[strnum] = malloc(strlen(options[i].))
				strnum++;
				in.string
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				switch(variables[options[i].readvar].type) {
					case 0:
						inputimg = variables[options[i].readvar].img;
						if(options[i].rw == 1) {
							vartochange = options[i].readvar;
							outputimg = variables[vartochange].img;
							redefvar = 1;
						}
						break;
					default:
						//printf("Variable has wrong type for function \"%s\" in line %d!!!\nVariable: \"%s\"\n", functionstr, linenum, variables[options[i].readvar].name);
						return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
				}
				break;
			default:
				//printf("Wrong type for function \"%s\" in line %d!!!\n", functionstr, linenum);
				return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
		}
	}
}*/

avtypes* get_options(func f, char* line, int end, var* variables, int varpos, int imgpos) {
	avtypes* err = malloc(sizeof(avtypes));
	err[0].type = -3;
	int len = rmleadingwhitespace(line, end);
	char* output;
	avtypes* arguments = malloc((f.arguments+2)*sizeof(avtypes));
	int o = 1;
	int notored3 = ~3;

	while(len < end) {
		int typ = checktype(line[len]);
		int offset;
		char* endptr;
		switch(typ) {
			case 1: {
				offset = getend(&line[len+1], line[len], end);
				char varname[offset+1];
				arguments[o].type = 1;
				arguments[o].string = malloc(offset+1);
				memcpy(&varname, &line[len+1], offset);
				varname[offset] = '\0';
				sprintf(arguments[o].string, varname);
				len += 2;
				break;
			}
			case 2: {
				offset = getnext(&line[len], end);
				arguments[o].type = 2;
				arguments[o].num = strtod(&line[len], &endptr);
				break;
			}
			case 3: {
				offset = getend(&line[len+1], '}', end);
				char varname[offset+1];
				arguments[o].type = 3;
				arguments[o].string = malloc(offset+1);
				memcpy(&varname, &line[len+1], offset);
				varname[offset] = '\0';
				sprintf(arguments[o].string, varname);
				len += 2;
				break;
			}
			default: {//check for img or variables
				offset = getnext(&line[len], end);
				char varname[offset+1];
				memcpy(&varname, &line[len], offset);
				varname[offset] = '\0';
				if(varname[0] == '$') {
					variables[varpos].type = 0;
					variables[varpos].name = malloc(offset+1);
					variables[varpos].img = imgpos;
					imgpos++;
					sprintf(variables[varpos].name, &varname[1]);
					arguments[o].type = 4;
					arguments[o].defvarimg = varpos;
					varpos++;
				} else if(varname[0] == '&') {
					int pos = cmpvariables(variables, varpos, &varname[1]);
					if(pos == -1) {
						//printf("Variable in line %d does not exist!!!\nVariable: \"%s\"\n", linenum, &varname[1]);
						err[0].err = make_error(4, "", 0, &varname[1], 0);
						return err;
					}
					arguments[o].type = 5;
					arguments[o].readvar = pos;
					arguments[o].rw = 1;
				} else {
					int pos = cmpvariables(variables, varpos, varname);
					if(pos == -1) {
						//printf("Variable in line %d does not exist!!!\nVariable: \"%s\"\n", linenum, varname);
						err[0].err = make_error(4, "", 0, varname, 0);
						return err;
					}
					arguments[o].type = 5;
					arguments[o].readvar = pos;
					arguments[o].rw = 0;
				}
			}
		}
		/*if(~((f.inputmask >> (o-1)*2) | notored3) != typ) {
			err[0].err = make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
			return err;
		}*/
		o++;
		len += offset;
		len += rmleadingwhitespace(&line[len], end);
	}
	arguments[0].type = -2;
	arguments[0].defvarimg = o;
	arguments[0].img = imgpos;
	arguments[0].readvar = varpos;
	return arguments;
}

error interprate(text t) {
//find way to insert this from generate_parser_txt.c
func functions[35] = {
	//name                              name_len  arg_amount  inputmask
	//./utils/rotate.h
	{"rotate_90",                        9,       1,          0},
	{"rotate_180",                       10,      1,          0},
	{"rotate_270",                       10,      1,          0},
	{"rotate_skew",                      11,      2,          512},
	{"rotate_rotation_matrix",           22,      2,          8192},
	//./utils/noise.h
	{"worley_noise_2d",                  15,      3,          704512},
	{"perlin_noise",                     12,      3,          45088768},
	//./utils/my_math.h
	//./utils/gui.h
	//./utils/scale.h
	{"box_sample",                       10,      2,          536870912},
	{"bilinear_interpolation",           22,      2,          2},
	//./utils/colors.h
	{"img_HSL_to_RGB",                   14,      1,          0},
	{"img_RGB_to_HSL",                   14,      1,          0},
	//./utils/misc.h
	{"clip",                             4,       3,          2688},
	{"select_subarray",                  15,      3,          61440},
	{"select_subarray_no_offset",        25,      3,          3932160},
	{"convolute_img",                    13,      2,          0},
	{"empty_img",                        9,       3,          2684354562},
	{"empty_colored_img",                17,      4,          936},
	//./utils/skew.h
	{"skew_horizontal",                  15,      2,          8192},
	{"skew_vertical",                    13,      2,          131072},
	//./utils/functions.h
	//./utils/parser.h
	//./utils/blur.h
	{"generate_box_sample_blur_kernel",  31,      2,          2621440},
	{"generate_gaussian_blur_kernel",    29,      3,          176160768},
	//./utils/mask.h
	{"rectangle_mask",                   14,      3,          4026531843},
	{"circle_mask",                      11,      3,          236},
	{"apply_mask",                       10,      4,          45056},
	//./utils/select.h
	{"selection_mask_between",           22,      3,          3932160},
	{"selection_mask_distance",          23,      3,          184549376},
	{"selection_mask",                   14,      2,          2147483648},
	{"selection_mask_rgb",               18,      2,          2},
	//./utils/display.h
	//./utils/pngtopng.h
	{"openimg",                          7,       1,          1},
	//./utils/wm.h
	//./utils/flip.h
	{"flip_horizontal",                  15,      1,          0},
	{"flip_vertical",                    13,      1,          0},
	//./utils/displaywm.h
	//./utils/posterize.h
	{"div_clip",                         8,       2,          512},
	//./utils/text.h
	//./utils/edge_detect.h
	{"sobel_accumulator_func",           22,      4,          179200},
	{"kirsch_accumulator_func",          23,      4,          45875200},
	{"kirsch_color_accumulator_func",    29,      4,          3154116610}
}; //--END--


	/*FILE* functionsfile = fopen("functions.txt", "r");
	char* functionline = NULL;
	size_t functionlinelen = 0;
	ssize_t functionread;
	int availablefunctionslen = 0;
	char c;
	char* endptr;
	for(c = getc(functionsfile); c != EOF; c = getc(functionsfile))
		if(c == '\n')
			availablefunctionslen++;*/
	//func availablefunctions[availablefunctionslen];
	int index = 0;
	/*fseek(functionsfile, 0, SEEK_SET);
	while((functionread = getline(&functionline, &functionlinelen, functionsfile)) != -1) {
		int len = functionread-2;
		for(int i = len-1; i > 0; i--) {
			if(functionline[i] == ' ') i = 0;
			len--;
		}
		len++;
		char funcname[len+1];
		memcpy(&funcname, functionline, len);
		funcname[len-1] = '\0';
		availablefunctions[index].arguments = (int)strtol(&functionline[len], &endptr, 10);
		availablefunctions[index].name = malloc(len+1);
		sprintf(availablefunctions[index].name, funcname);
		index++;
	}
	fclose(functionsfile);*/

	int varamount = 0;
	int varpos = 0;
	for(int i = 0; i < t.len; i++)
		if(t.string[i] == '$')
			varamount++;
	var variables[varamount];
	My_png images[varamount];
	int imgpos = 0;

	char* line = NULL;
	size_t linelen = 0;
	ssize_t read;
	int imgamount = 0;
	int o = 0;
	char* string = malloc(t.len);
	strcpy(string, t.string);
	for(int linenum = 1; linenum <= t.lines; linenum++) {
		int end = t.linelens[linenum-1];
		line = &string[o];
		o += t.linelens[linenum-1];

		for(int i = 0; i < end; i++)
			if(line[i] == '#')
				end = i;
		//printf("|%.*s|  |%*s| %d %c\n", end, line, t.linelens[linenum-1]-1, line, end, line[end]);
		end = rmwhitespace(line, end);
		if(end > 1) {
			//printf("|%.*s|  %d\n", end, line, end);
			if(line[0] == '$') {
				int varlen = getnext(line, end);
				char varname[varlen]; //change this
				memcpy(&varname, &line[1], varlen-1);
				varname[varlen-1] = '\0';
				variables[varpos].name = malloc(varlen);
				sprintf(variables[varpos].name, varname);
				//printf("var: |%s|\n", varname);
				int offset = rmleadingwhitespace(&line[varlen], end-varlen)+varlen;
				if(offset == varlen) {
					//printf("Variable not defined in line %d!!!\nVariable: \"%s\"\n", linenum, varname);
					return make_error(1, "", linenum, varname, 0);
				}
				for(int i = offset; i < end; i++) {
					if(!(line[i] == ' ')) {
						offset = i;
						i = end;
					}
				}

				int vallen;
				variables[varpos].type = checktype(line[offset]);
				switch(variables[varpos].type) {
					case 1:
						offset++;
						vallen = getend(&line[offset], line[offset-1], end)+1;
						end = offset+vallen;
						break;
					case 3:
						offset++;
						vallen = getend(&line[offset], '}', end)+1;
						end = offset+vallen;
						break;
					default:
						vallen = end-offset+1;
				}

				char varval[vallen]; //change this
				memcpy(&varval, &line[offset], vallen-1);
				varval[vallen-1] = '\0';
				variables[varpos].value = malloc(vallen);
				sprintf(variables[varpos].value, varval);
				varpos++;
			} else {
				int functionstrlen = getnext(line, end);
				char functionstr[functionstrlen+1];
				memcpy(&functionstr, line, functionstrlen*sizeof(char));
				functionstr[functionstrlen] = '\0';
				int selectedfunction = -1;
				for(int i = 0; i < functionslen; i++) {
					if(strcmp(functions[i].name, functionstr) == 0) {
						selectedfunction = i;
						//break;
						i = availablefunctionslen;
					}
				}
				avtypes* options = get_options(
					availablefunctions[selectedfunction],
					&line[functionstrlen],
					end-functionstrlen,
					variables,
					varpos,
					imgpos
				);
				if(options[0].type == -3) {
					error err = options[0].err;
					err.linenum = linenum;
					return err;
				}
				int o = options[0].defvarimg;
				varpos = options[0].readvar;
				int vartochange = varpos;
				int redefvar = 0;
				int inputimg;
				int outputimg = imgpos;
				imgpos = options[0].img;
				My_png img;
				int argpos = 0;
				switch(selectedfunction) {
					case 0:
						printf("openimg\n");
						char* f;
						for(int i = 1; i < o; i++) {
							switch(options[i].type) {
								case 1:
									f = options[i].string;
									break;
								case 4:
									break;
								case 5:
									switch(variables[options[i].readvar].type) {
										case 1:
											f = variables[options[i].readvar].value;
											if(options[i].rw == 1) {
												vartochange = options[i].readvar;
												outputimg = variables[vartochange].img;
												redefvar = 1;
											}
											break;
										default:
											//printf("Variable has wrong type for function \"%s\" in line %d!!!\nVariable: \"%s\"\n", functionstr, linenum, variables[options[i].readvar].name);
											return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
									}
									break;
								default:
									//printf("Wrong type for function \"%s\" in line %d!!!\n", functionstr, linenum);
									return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
							}
						}
						if(o > availablefunctions[selectedfunction].arguments+1 || redefvar == 1) img = openimg(f);
						break;
					case 1:
						printf("rotate_90\n");
						for(int i = 1; i < o; i++) {
							switch(options[i].type) {
								case 4:
									break;
								case 5:
									switch(variables[options[i].readvar].type) {
										case 0:
											inputimg = variables[options[i].readvar].img;
											if(options[i].rw == 1) {
												vartochange = options[i].readvar;
												outputimg = variables[vartochange].img;
												redefvar = 1;
											}
											break;
										default:
											//printf("Variable has wrong type for function \"%s\" in line %d!!!\nVariable: \"%s\"\n", functionstr, linenum, variables[options[i].readvar].name);
											return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
									}
									break;
								default:
									//printf("Wrong type for function \"%s\" in line %d!!!\n", functionstr, linenum);
									return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
							}
						}
						if(o > availablefunctions[selectedfunction].arguments+1 || redefvar == 1) img = rotate_90(images[inputimg]);
						break;
					case 2:
						printf("rotate_180\n");
						break;
					case 3:
						printf("rotate_270\n");
						break;
					case 4:
						printf("rotate_skew\n");
						break;
					case 5:
						printf("rotate_rotation_matrix\n");
						double angle;
						for(int i = 1; i < o; i++) {
							switch(options[i].type) {
								case 2:
									angle = options[i].num;
									break;
								case 4:
									break;
								case 5:
									switch(variables[options[i].readvar].type) {
										case 0:
											inputimg = options[i].img;
											if(options[i].rw == 1) {
												vartochange = options[i].readvar;
												outputimg = variables[vartochange].img;
												redefvar = 1;
											}
											break;
										case 2:
											angle = strtod(variables[options[i].readvar].value, &f);
											break;
										default:
											//printf("Variable has wrong type for function \"%s\" in line %d!!!\nVariable: \"%s\"\n", functionstr, linenum, variables[options[i].readvar].name);
											return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
									}
									break;
								default:
									//printf("Wrong type for function \"%s\" in line %d!!!\n", functionstr, linenum);
									return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
							}
						}
						if(o > availablefunctions[selectedfunction].arguments+1 || redefvar == 1) img = rotate_rotation_matrix(images[inputimg], angle);
					break;
					case 6:
						printf("box_sample\n");
						break;
					case 7:
						printf("bilinear_interpolation\n");
						break;
					case 8:
						printf("select_subarray\n");
						break;
					case 9:
						printf("convolute_img\n");
						break;
					case 10:
						printf("convolute_img_multiple_kernels\n");
						break;
					case 11:
						printf("empty_img\n");
						break;
					case 12:
						printf("skew_horizontal\n");
						break;
					case 13:
						printf("skew_vertical\n");
						break;
					case 14:
						printf("generate_box_sample_blur_kernel\n");
						break;
					case 15:
						printf("generate_gaussian_blur_kernel\n");
						break;
					case 16:
						printf("rectangle_mask\n");
						break;
					case 17:
						printf("circle_mask\n");
						float radius;
						char* arrays[2];
						for(int i = 1; i < o; i++) {
							switch(options[i].type) {
								case 2:
									radius = options[i].num;
									break;
								case 3:
									arrays[argpos] = options[i].string;
									argpos++;
									break;
								case 4:
									break;
								case 5:
									switch(variables[options[i].readvar].type) {
										case 2:
											radius = strtof(variables[options[i].readvar].value, &f);
											break;
										case 3:
											arrays[argpos] = variables[options[i].readvar].value;
											break;
										default:
											//printf("Variable has wrong type for function \"%s\" in line %d!!!\nVariable: \"%s\"\n", functionstr, linenum, variables[options[i].readvar].name);
											return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
									}
									break;
								default:
									//printf("Wrong type for function \"%s\" in line %d!!!\n", functionstr, linenum);
									return make_error(2, functionstr, linenum, variables[options[i].readvar].name, 0);
							}
						}
						if(o > availablefunctions[selectedfunction].arguments+1 || redefvar == 1) img = circle_mask(arraytoxyz_int(arrays[0]), radius, arraytoxy_int(arrays[1]));
						break;
					case 18:
						printf("apply_mask\n");
						break;
					case 19:
						printf("flip_horizontal\n");
						break;
					case 20:
						printf("flip_vertical\n");
						break;
					case 21:
						printf("display_all\n");
						int i = 11;
						int amount = 0;
						while(line[i] != '\0') {
							if(
								(line[i] == ' ' || line[i] == '\t') &&
								(line[i+1] != ' ' || line[i+1] != '\t')
							) {
								amount++;
							}
							i++;
						}
						break;
					default:
						//printf("Function in line %d does not exist!!!\nFunction: \"%s\"\n", linenum, functionstr);
						return make_error(3, functionstr, linenum, "", 0);
				}
				if(o > availablefunctions[selectedfunction].arguments+1 || redefvar == 1) {
					if(redefvar == 0) {
						variables[vartochange].type = 0;
						variables[vartochange].img = imgpos;
						images[outputimg] = img;
						//imgpos++;
					} else {
						free(images[variables[vartochange].img].image);
						images[outputimg] = img;
					}
				}
				printf("img type: %d\n", images[outputimg].type);
			}
		}
	}
	//clear_window(display, w, win);
	//display_all_list(display, w, gc, win, imgpos, images);
	error err;
	err.type = 0;
	err.images.len = imgpos;
	err.images.list = malloc(sizeof(My_png)*imgpos);
	err.images.list = memcpy(err.images.list, images, sizeof(My_png)*imgpos);
	return err;
}
