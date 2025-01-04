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

#include "pngtopng.h"
//#include "stl/load_stl.h"
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
#include "select.h"

enum {
	TYPE_ERROR = -3,
	TYPE_INFO,
	TYPE_UNKOWN,
	TYPE_IMG,
	TYPE_STRING,
	TYPE_NUM,
	TYPE_ARRAY,
	TYPE_DEFVARIMG,
	TYPE_READVAR
};

enum {
	START_ARRAY = '{',
	END_ARRAY = '}',
	START_ITEM = '[',
	END_ITEM = ']',
	START_NUM = '0',
	END_NUM = '9',
	SPACE = ' ',
	NEW_LINE = '\n',
	STRING_1 = '\"',
	STRING_2 = '\'',
	COMMENT = '#',
	DEF_VAR = '$',
	REDEF_VAR = '&'
};

int rmwhitespace(char* line, int start) {
	for(int i = start-1; i > 0; i--)
		if(!(line[i] == SPACE || line[i] == NEW_LINE))
			return i+1;
	return 0;
}

int rmleadingwhitespace(char* line, int end) {
	for(int i = 0; i < end; i++)
		if(!(line[i] == SPACE || line[i] == NEW_LINE))
			return i;
	return end;
}

int getnext(char* line, int end) {
	for(int i = 0; i < end; i++)
		if(line[i] == SPACE || line[i] == NEW_LINE)
			return i;
	return end;
}

int getend(char* line, char delim, int end) {
	for(int i = 0; i < end; i++)
		if(line[i] == delim)
			return i;
	return end;
}

int getstart(char* line, char delim, int start) {
	for(int i = start-1; 0 < i; i--)
		if(line[i] == delim)
			return i;
	return 0;
}

int getarrayend(char* line, int end) {
	int nested = 0;
	for(int i = 0; i < end; i++) {
		if(line[i] == START_ARRAY) {
			nested++;
		} else if(line[i] == END_ARRAY) {
			nested--;
			if(!nested) return i-1;
		}
	}
}

xyz_int arraytoxyz_int(localvarlist variables, unsigned int i) {
	//char* endptr;
	xyz_int pos;
	//pos.x = strtol(array, &endptr, 10);
	//pos.y = strtol(endptr, &endptr, 10);
	//pos.z = strtol(endptr, &endptr, 10);
	pos.x = variables.v[variables.v[i].avi[0]].num;
	pos.y = variables.v[variables.v[i].avi[1]].num;
	pos.z = variables.v[variables.v[i].avi[2]].num;
	return pos;
}

xy_int arraytoxy_int(localvarlist variables, unsigned int i) {
//xy_int arraytoxy_int(char* array) {
	//char* endptr;
	xy_int pos;
	//pos.x = strtol(array, &endptr, 10);
	//pos.y = strtol(endptr, &endptr, 10);
	pos.x = variables.v[variables.v[i].avi[0]].num;
	pos.y = variables.v[variables.v[i].avi[1]].num;
	return pos;
}

xy arraytoxy(localvarlist variables, unsigned int i) {
//xy arraytoxy(char* array) {
	//char* endptr;
	xy pos;
	//pos.x = strtof(array, &endptr);
	//pos.y = strtof(endptr, &endptr);
	pos.x = variables.v[variables.v[i].avi[0]].num;
	pos.y = variables.v[variables.v[i].avi[1]].num;
	return pos;
}

//polygon here pls

color arraytocolor(localvarlist variables, unsigned int i) {
	color c;
	c.type = variables.v[variables.v[i].avi[0]].num;
	c.c = malloc(color_to_depth(c.type)*sizeof(pval));
	for(unsigned int a = 1; a < variables.v[i].avilen; a++) c.c[a] = variables.v[variables.v[i].avi[a]].num;
	return c;
}

pval* arraytopvalarray(localvarlist variables, unsigned int i) {
	pval* array = malloc(variables.v[i].avilen*sizeof(pval));
	for(unsigned int a = 0; a < variables.v[i].avilen; a++) array[a] = variables.v[variables.v[i].avi[a]].num;
	return array;
}

int checktype(char varval) {
	if(varval == STRING_1 || varval == STRING_2) {
		return TYPE_STRING;
	} else if(varval >= START_NUM && varval <= END_NUM) {
		return TYPE_NUM;
	} else if(varval == START_ARRAY) {
		return TYPE_ARRAY;
	}
	return TYPE_UNKOWN;
}

int cmpvariables(localvarlist variables, char* cmpvar) {
	for(int i = 0; i < variables.vlen; i++)
		if(variables.v[i].userdef == 1) //rm 1
			if(strcmp(cmpvar, variables.v[i].name) == 0)
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
			err.function = malloc(strlen(function)+1);
			strcpy(err.function, function);
		case 4:
		case 1:
			err.variable = malloc(strlen(variable)+1);
			strcpy(err.variable, variable);
			break;
		case 3:
			err.function = malloc(strlen(function)+1);
			strcpy(err.function, function);
			break;
		default:
			err.arraylen = arraylen;
			err.variable = malloc(strlen(variable)+1);
			strcpy(err.variable, variable);
			err.function = malloc(strlen(function)+1);
			strcpy(err.function, function);
	}
	err.linenum = linenum;
	return err;
}

void free_defvarerr(defvarerr err) {
	//free(err->vi);
	//free(err->vd);
	//free(err);
	if(err.len != 0) free(err.vi);
	if(err.vdlen != 0) free(err.vd);
}

void realloc_defvarerr_vi(defvarerr* err, unsigned int sizevi) {
	if(err->len == 0 ) {
		err->vi = malloc(sizeof(int)*sizevi);
	} else {
		err->vi = realloc(err->vi, sizeof(int)*sizevi);
	}
	err->len = sizevi;
}

void realloc_defvarerr_vd(defvarerr* err, unsigned int sizevd) {
	if(err->vdlen == 0 ) {
		err->vd = malloc(sizeof(int)*sizevd);
	} else {
		err->vd = realloc(err->vd, sizeof(int)*sizevd);
	}
	err->vdlen = sizevd;
}

void initialize_variable(localvarlist* variables, unsigned int pos) {
	variables->v[pos].type = TYPE_UNKOWN;
	variables->v[pos].name = NULL;
	//variables->v[pos].userdef = 0;
	variables->v[pos].string = NULL;
	//variables->v[pos].num = 0;
	variables->v[pos].avi = NULL;
	//variables->v[pos].avilen = 0;
	//variables->v[pos].img = 0;
	//variables->v[pos].self = 0;
}

void realloc_variables(localvarlist* variables, unsigned int size) {
	if(variables->vlen == 0) {
		variables->v = malloc(sizeof(var)*size);
	} else {
		variables->v = realloc(variables->v, sizeof(var)*size);
		initialize_variable(variables, size-1);
	}
	variables->vlen = size;
}

void initialize_defvarerr(defvarerr* err) {
	err->type = TYPE_UNKOWN;
	err->len = 0;
	err->vi = NULL;
	err->uv = 0;
	err->vd = NULL;
	err->vdlen = 0;
}

unsigned int dump_vars_inner(localvarlist variables, unsigned int i, unsigned short tabs) {
	switch(variables.v[i].type) {
		case TYPE_ERROR:
			printf("%*serror\n", tabs, "");
			break;
		case TYPE_INFO:
			printf("%*sinfo\n", tabs, "");
			break;
		case TYPE_UNKOWN:
			printf("%*sunkown\n", tabs, "");
			if(variables.v[i].userdef) printf("%*s%s\n", tabs+3, "", variables.v[i].name);
			break;
		case TYPE_IMG:
			printf("%*simg\n", tabs, "");
			if(variables.v[i].userdef) printf("%*s%s\n", tabs+3, "", variables.v[i].name);
			printf("%*s%d\n", tabs+3, "", variables.v[i].img);
			break;
		case TYPE_STRING:
			printf("%*sstring\n", tabs, "");
			if(variables.v[i].userdef) printf("%*s%s\n", tabs+3, "", variables.v[i].name);
			printf("%*s%s\n", tabs+3, "", variables.v[i].string);
			break;
		case TYPE_NUM:
			printf("%*snum\n", tabs, "");
			if(variables.v[i].userdef) printf("%*s%s\n", tabs+3, "", variables.v[i].name);
			printf("%*s%f\n", tabs+3, "", variables.v[i].num);
			break;
		case TYPE_ARRAY:
			printf("%*sarray\n", tabs, "");
			if(variables.v[i].userdef) printf("%*s%s\n", tabs+3, "", variables.v[i].name);
			printf("%*s{", tabs+3, "");
			for(int x = 0; x < variables.v[i].avilen-1; x++) printf("%d ", variables.v[i].avi[x]);
			printf("%d}\n", variables.v[i].avi[variables.v[i].avilen-1]);
			break;
		case TYPE_DEFVARIMG:
			printf("%*sdefvarimg\n", tabs, "");
			break;
		case TYPE_READVAR:
			printf("%*sreadvar\n", tabs, "");
			break;
	}
	return i-1;
}

void dump_vars(localvarlist variables) {
	printf("%d\n", variables.vlen);
	printf("------------------------------------------------\n");
	int index = 0;
	for(int i = variables.vlen-1; i >= 0; i--)  {
		printf("%*s%d %d %d\n", 3, "", variables.v[i].type, i, variables.v[i].self);
		index = dump_vars_inner(variables, i, 4);
	}
	printf("------------------------------------------------\n\n");
}

polygon arraytopolygon(localvarlist variables, unsigned int i) {
	polygon p;
	p.corners = variables.v[i].avilen;
	p.p = malloc(p.corners*sizeof(xy_int));
	for(unsigned int j = 0; j < p.corners; j++)
		p.p[j] = arraytoxy_int(variables, variables.v[i].avi[j]);
	return p;
}

defvarerr define_vars(char* line, int end, localvarlist* variables) {
	printf("|%.*s|\n", end, line);
	int len = rmleadingwhitespace(line, end);
	defvarerr err;
	initialize_defvarerr(&err);
	err.type = 0;
	while(len < end) {
		int type = checktype(line[len]);
		int offset = 0;
		int pos;
		char* endptr;
		switch(type) {
			case TYPE_STRING: //string
				offset = getend(&line[len+1], line[len], end);
				pos = variables->vlen;
				realloc_defvarerr_vi(&err, err.len+1);
				err.vi[err.len-1] = pos;
				realloc_variables(variables, variables->vlen+1);
				variables->v[pos].userdef = 0;
				variables->v[pos].type = 1;
				variables->v[pos].self = pos;
				variables->v[pos].string = malloc(offset+1);
				variables->v[pos].string = memcpy(variables->v[pos].string, &line[len+1], offset);
				variables->v[pos].string[offset] = '\0';
				printf("string: |%s|\n", variables->v[pos].string);
				len += 2;
				break;
			case TYPE_NUM: //num
				offset = getnext(&line[len], end);
				pos = variables->vlen;
				realloc_defvarerr_vi(&err, err.len+1);
				err.vi[err.len-1] = pos;
				realloc_variables(variables, variables->vlen+1);
				variables->v[pos].userdef = 0;
				variables->v[pos].type = 2;
				variables->v[pos].self = pos;
				variables->v[pos].num = strtod(&line[len], &endptr);
				printf("num: %.3f %d %d\n", variables->v[pos].num, err.len, pos);
				break;
			case TYPE_ARRAY: //array
				offset = getarrayend(&line[len], end);
				printf("|%.*s| %d %d\n", offset, &line[len+1], end, offset);
				defvarerr avi = define_vars(&line[len+1], offset, variables);
				printf("%d %d\n\n", avi.type, avi.vdlen);
				if(avi.type != 0) {
					return avi;
				} else if(avi.vdlen != 0) {
					avi.type = 6;
					return avi;
				}
				pos = variables->vlen;
				realloc_defvarerr_vi(&err, err.len+1);
				err.vi[err.len-1] = pos;
				realloc_variables(variables, variables->vlen+1);
				variables->v[pos].userdef = 0;
				variables->v[pos].type = 3;
				variables->v[pos].self = pos;
				variables->v[pos].avi = avi.vi;
				variables->v[pos].avilen = avi.len;
				if(avi.vdlen) free(avi.vd);
				//err = realloc(err, errlen*sizeof(defvarerr));
				offset += 2;
				printf("err info: %d %d %d %d\n", err.type, err.len, err.vdlen, err.uv);
				break;
			default:
				offset = getnext(&line[len], end-len);
				char varname[offset+1];
				memcpy(&varname, &line[len], offset);
				varname[offset] = '\0';
				pos = variables->vlen;
				printf("var: |%s| %d %d\n", varname, offset, end);
				int arraypos;
				short hasindex = 0;
				if(varname[offset-1] == END_ITEM) {
					int bracketstart = getstart(varname, START_ITEM, offset);
					if(bracketstart == 0) {
						err.type = 7;
						return err;
					}
					arraypos = strtol(&varname[bracketstart], &endptr, 10);
					varname[bracketstart] = '\0';
					hasindex = 1;
				}
				if(line[len] == DEF_VAR) { //fucky malloc stuff at circle_mask
					err.uv++;
					printf("var\n");
					realloc_defvarerr_vd(&err, err.vdlen+1);
					printf("var\n");
					err.vd[err.vdlen-1] = pos;
					realloc_variables(variables, variables->vlen+1);
					variables->v[pos].userdef = 1;
					variables->v[pos].type = TYPE_UNKOWN;
					variables->v[pos].self = pos;
					variables->v[pos].name = malloc(offset+1);
					memcpy(variables->v[pos].name, &varname[1], offset);
					variables->v[pos].name[offset] = '\0';
					printf("var name: |%s|\n", variables->v[pos].name);
				} else if(varname[0] == REDEF_VAR) {
					int foundvar = cmpvariables(*variables, &varname[1]);
					if(foundvar == -1) {
						err.type = 4;
						return err;
					}
					printf("var\n");
					realloc_defvarerr_vd(&err, err.vdlen+1);
					printf("var\n");
					realloc_defvarerr_vi(&err, err.len+1);
					if(variables->v[foundvar].type == TYPE_ARRAY && hasindex == 1) {
						err.vi[err.len-1] = variables->v[variables->v[foundvar].avi[arraypos]].self;
						err.vd[err.vdlen-1] = variables->v[variables->v[foundvar].avi[arraypos]].self;
					} else {
						err.vi[err.len-1] = variables->v[foundvar].self;
						err.vd[err.vdlen-1] = variables->v[foundvar].self;
					}
				} else {
					int foundvar = cmpvariables(*variables, varname);
					if(foundvar == -1) {
						err.type = 4;
						return err;
					}
					realloc_defvarerr_vi(&err, err.len+1);
					if(variables->v[foundvar].type == TYPE_ARRAY && hasindex == 1) {
						err.vi[err.len-1] = variables->v[variables->v[foundvar].avi[arraypos]].self;
					} else {
						err.vi[err.len-1] = variables->v[foundvar].self;
					}
					printf("found var:\n");
					dump_vars_inner(*variables, err.vi[err.len-1], 5);
				}
		}
		len += offset;
		len += rmleadingwhitespace(&line[len], end);
		printf("len: %d\n", len);
	}
	err.type = 0;
	return err;
}

avtypes get_options(func f, char* line, int end, localvarlist* variables) {
	avtypes err;
	err.type = -3;
	unsigned int crvlen = 0;

	avtypes arguments;
	arguments.v = malloc(sizeof(int)*f.arguments);
	unsigned int vi = 0; //if not equal to f.arguments -> return err
	arguments.cvi = malloc(sizeof(int)*f.outputs);
	unsigned int cvi = 0; //if not equal to f.outputs -> return err

	//defvarerr* ivi = malloc(sizeof(defvarerr));
	//initialize_defvarerr(ivi, 0);
	defvarerr ivi = define_vars(line, end, variables);

	printf("ivi info: %d %d %d %d\n", ivi.type, ivi.len, ivi.vdlen, ivi.uv);

	if(ivi.type != 0) {
		err.err = make_error(ivi.type, f.name, 0, "", 0);
		return err;
	} else if(f.arguments != ivi.len) {
		err.err = make_error(8, f.name, 0, "", 0);
		return err;
	} else if(f.outputs != ivi.vdlen) {
		err.err = make_error(9, f.name, 0, "", 0);
		return err;
	}
	for(int i = 0; i < ivi.len-ivi.uv; i++) {
		if(~((~(f.inputmask >> i*2)) | (~3)) != variables->v[ivi.vi[i]].type) {
			err.err = make_error(2, f.name, 0, variables->v[ivi.vi[i]].name, 0);
			return err;
		}
	}
	arguments.v = ivi.vi;
	arguments.cvi = ivi.vd;
	arguments.uv = ivi.uv;
	//free(ivi);
	printf("got options!\n");
	return arguments;
}

error interprate(text t) {
	func functions[38] = { 
		//name                              name_len  arg_amount  inputmask
		//./utils/posterize.h
		{"div_clip",                        8,        2,          8, 1, 0},
		//./utils/colors.h
		{"img_HSL_to_RGB",                  14,       1,          0, 1, 0},
		{"img_RGB_to_HSL",                  14,       1,          0, 1, 0},
		//./utils/noise.h
		{"worley_noise_2d",                 15,       3,          43, 1, 0},
		{"perlin_noise",                    12,       3,          43, 1, 0},
		//./utils/pngtopng.h
		{"openimg",                         7,        1,          1, 1, 0},
		{"save_image",                      10,       2,          4, 1, 0},
		//./utils/text.h
		//./utils/skew.h
		{"skew_horizontal",                 15,       2,          8, 1, 0},
		{"skew_vertical",                   13,       2,          8, 1, 0},
		//./utils/mask.h
		{"rectangle_mask",                  14,       3,          63, 1, 0},
		{"circle_mask",                     11,       3,          59, 1, 0},
		{"apply_mask",                      10,       4,          176, 1, 0},
		{"polygon_mask",                    12,       2,          11, 1, 0},
		//./utils/gui.h
		//./utils/flip.h
		{"flip_horizontal",                 15,       1,          0, 1, 0},
		{"flip_vertical",                   13,       1,          0, 1, 0},
		//./utils/select.h
		{"selection_mask_between",          22,       3,          60, 1, 0},
		{"selection_mask_distance",         23,       3,          44, 1, 0},
		{"selection_mask_recursive_distance", 33,       4,          236, 1, 0},
		{"selection_mask_raytrace",         23,       4,          188, 1, 0},
		//./utils/blur.h
		{"generate_box_sample_blur_kernel", 31,       2,          10, 1, 0},
		{"generate_gaussian_blur_kernel",   29,       3,          42, 1, 0},
		//./utils/scale.h
		{"box_sample",                      10,       2,          8, 1, 0},
		{"bilinear_interpolation",          22,       2,          8, 1, 0},
		//./utils/edge_detect.h
		{"sobel_accumulator_func",          22,       4,          175, 1, 0},
		{"kirsch_accumulator_func",         23,       4,          175, 1, 0},
		{"kirsch_color_accumulator_func",   29,       4,          175, 1, 0},
		//./utils/rotate.h
		{"rotate_90",                       9,        1,          0, 1, 0},
		{"rotate_180",                      10,       1,          0, 1, 0},
		{"rotate_270",                      10,       1,          0, 1, 0},
		{"rotate_skew",                     11,       2,          8, 1, 0},
		{"rotate_rotation_matrix",          22,       2,          8, 1, 0},
		//./utils/wm.h
		//./utils/my_math.h
		//./utils/backupparser2.h
		//./utils/misc.h
		{"clip",                            4,        3,          42, 1, 0},
		{"select_subarray",                 15,       3,          15, 1, 0},
		{"select_subarray_no_offset",       25,       3,          15, 1, 0},
		{"convolute_img",                   13,       2,          0, 1, 0},
		{"empty_img",                       9,        3,          42, 1, 0},
		{"empty_colored_img",               17,       4,          234, 1, 0},
		{"regular_plygon",                  14,       3,          42, 1, 0},
		//./utils/parser.h
		//./utils/displaywm.h
		//./utils/xdg-shell-client-protocol.
	}; //--END--
	int functionslen = 38;
	localvarlist variables;
	//variables.v = malloc(sizeof(var));
	variables.v = NULL;
	variables.vlen = 0;
	My_png_list images;
	images.list = malloc(sizeof(My_png));
	images.len = 0;

	char* line = NULL;
	size_t linelen = 0;
	ssize_t read;
	int o = 0;
	char* string = malloc(t.len);
	memcpy(string, t.string, t.len);
	for(int linenum = 1; linenum <= t.lines; linenum++) {
		int end = t.linelens[linenum-1];
		line = &string[o];
		o += t.linelens[linenum-1];

		for(int i = 0; i < end; i++)
			if(line[i] == COMMENT)
				end = i;
		//printf("|%.*s|  |%.*s| %d %c\n", end, line, t.linelens[linenum-1]-1, line, end, line[end]);
		end = rmwhitespace(line, end);
		//printf("|%.*s|  |%.*s| %d %c\n", end, line, t.linelens[linenum-1]-1, line, end, line[end]);
		if(end > 1) {
			printf("line: |%.*s|  %d\n", end, line, end);
			if(line[0] == DEF_VAR) { //doesn't consider there being spaces infront of '$'
				line = &line[1];
				int varlen = getnext(line, end);
				int offset = rmleadingwhitespace(&line[varlen], end-varlen)+varlen;
				if(offset == varlen) {
					line[varlen+1] = '\0';
					return make_error(1, "", linenum, &line[1], 0);
				}
				//defvarerr* newvar = malloc(sizeof(defvarerr));
				//initialize_defvarerr(newvar, 0);
				//define_vars(&line[offset], end-offset-1, &variables, newvar, 0); //check for already existing var
				//printf("newvar: |%.*s|%d\n", varlen, line, variables.vlen); //within define_vars array reallocing wipes everything in newvar. i don't know why

				//defvarerr* newvar = malloc(sizeof(defvarerr));
				//initialize_defvarerr(newvar, 0);
				defvarerr newvar = define_vars(&line[offset], end-offset-1, &variables);
				printf("newvar info: %d %d %d %d\n", newvar.type, newvar.len, newvar.vdlen, newvar.uv);

				unsigned int vp = variables.vlen-1;
				variables.v[vp].userdef = 1;
				variables.v[vp].name = malloc(varlen+1);
				variables.v[vp].name = memcpy(variables.v[vp].name, line, varlen);
				variables.v[vp].name[varlen] = '\0';
				printf("print type\n");
				printf("var: |%s|%hi\n", variables.v[vp].name, newvar.type); //shorts make an "Invalid read of size 2"in valgrind
				if(newvar.type != 0) {
					return make_error(newvar.type, "", linenum, variables.v[vp].name, 0);
				}
				printf("var: |%s|\n", variables.v[vp].name);
				dump_vars(variables);
				free_defvarerr(newvar);
				//free(newvar);
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
						i = functionslen;
					}
				}
				avtypes options = get_options(
					functions[selectedfunction],
					&line[functionstrlen],
					end-functionstrlen,
					&variables
				);
				dump_vars(variables);
				if(options.type == -3) {
					options.err.linenum = linenum;
					return options.err;
				}
				int outputimg;
				short freeimg = 0;
				My_png* imgptr = NULL;
				for(int i = 0; i < functions[selectedfunction].outputs; i++) {
					printf("output type %d: ", functions[selectedfunction].outputmask);
					switch(~((~(functions[selectedfunction].outputmask >> i*2)) | (~3))) {
						case TYPE_IMG:
							printf("image\n");
							printf("%d\n", options.uv);
							printf("%d\n", i);
							printf("%d\n", options.cvi[i]);
							printf("%d\n", variables.v[options.cvi[i]].type);
							printf("\n");
							if(options.uv || variables.v[options.cvi[i]].type) {
								images.len++;
								images.list = realloc(images.list, images.len*sizeof(My_png));
								outputimg = images.len-1;
							} else {
								printf("redefine img var!\n");
								outputimg = variables.v[options.cvi[i]].img;
								freeimg = 1;
								imgptr = &images.list[outputimg];
							}
							printf("var stuff done\n");
							variables.v[options.cvi[i]].type = 0;
							variables.v[options.cvi[i]].img = outputimg;
							break;
						case TYPE_STRING:
							printf("string\n");
							break;
						case TYPE_NUM:
							printf("num\n");
							break;
						case TYPE_ARRAY:
							printf("array\n");
							break;
						default:
							return make_error(10, functions[selectedfunction].name, linenum, "", 0);
					}
				}
				switch(selectedfunction) { //--startswitch--
					case 0:
						printf("div_clip\n");
						images.list[outputimg] = div_clip(images.list[variables.v[options.v[0]].img], (int)variables.v[options.v[1]].num);
						break;
					case 1:
						printf("img_HSL_to_RGB\n");
//);
						break;
					case 2:
						printf("img_RGB_to_HSL\n");
//);
						break;
					case 3:
						printf("worley_noise_2d\n");
						images.list[outputimg] = worley_noise_2d(arraytoxyz_int(variables, options.v[0]), (int)variables.v[options.v[1]].num, (int)variables.v[options.v[2]].num);
						break;
					case 4:
						printf("perlin_noise\n");
						images.list[outputimg] = perlin_noise(arraytoxyz_int(variables, options.v[0]), (int)variables.v[options.v[1]].num, (int)variables.v[options.v[2]].num);
						break;
					case 5:
						printf("openimg\n");
						images.list[outputimg] = openimg(variables.v[options.v[0]].string);
						break;
					case 6:
						printf("save_image\n");
////images.list[variables.v[options.v[0]].img], variables.v[options.v[1]].string);
						break;
					case 7:
						printf("skew_horizontal\n");
						images.list[outputimg] = skew_horizontal(images.list[variables.v[options.v[0]].img], variables.v[options.v[1]].num);
						break;
					case 8:
						printf("skew_vertical\n");
						images.list[outputimg] = skew_vertical(images.list[variables.v[options.v[0]].img], variables.v[options.v[1]].num);
						break;
					case 9:
						printf("rectangle_mask\n");
						images.list[outputimg] = rectangle_mask(arraytoxyz_int(variables, options.v[0]), arraytoxy_int(variables, options.v[1]), arraytoxy_int(variables, options.v[2]));
						break;
					case 10:
						printf("circle_mask\n");
						images.list[outputimg] = circle_mask(arraytoxyz_int(variables, options.v[0]), variables.v[options.v[1]].num, arraytoxy_int(variables, options.v[2]));
						break;
					case 11:
						printf("apply_mask\n");
						images.list[outputimg] = apply_mask(images.list[variables.v[options.v[0]].img], images.list[variables.v[options.v[1]].img], arraytoxyz_int(variables, options.v[2]), (int)variables.v[options.v[3]].num);
						break;
					case 12:
						printf("polygon_mask\n");
						images.list[outputimg] = polygon_mask(arraytopolygon(variables, options.v[0]), (int)variables.v[options.v[1]].num);
						printf("donded polygon_mask\n");
						break;
					case 13:
						printf("flip_horizontal\n");
						images.list[outputimg] = flip_horizontal(images.list[variables.v[options.v[0]].img]);
						break;
					case 14:
						printf("flip_vertical\n");
						images.list[outputimg] = flip_vertical(images.list[variables.v[options.v[0]].img]);
						break;
					case 15:
						printf("selection_mask_between\n");
						images.list[outputimg] = selection_mask_between(images.list[variables.v[options.v[0]].img], arraytocolor(variables, options.v[1]), arraytocolor(variables, options.v[2]));
						break;
					case 16:
						printf("selection_mask_distance\n");
						images.list[outputimg] = selection_mask_distance(images.list[variables.v[options.v[0]].img], arraytocolor(variables, options.v[1]), variables.v[options.v[2]].num);
						break;
					case 17:
						printf("selection_mask_recursive_distance\n");
						images.list[outputimg] = selection_mask_recursive_distance(images.list[variables.v[options.v[0]].img], arraytocolor(variables, options.v[1]), variables.v[options.v[2]].num, arraytoxy_int(variables, options.v[3]));
						break;
					case 18:
						printf("selection_mask_raytrace\n");
						images.list[outputimg] = selection_mask_raytrace(images.list[variables.v[options.v[0]].img], arraytocolor(variables, options.v[1]), arraytoxy_int(variables, options.v[2]), (int)variables.v[options.v[3]].num);
						break;
					case 19:
						printf("generate_box_sample_blur_kernel\n");
						images.list[outputimg] = generate_box_sample_blur_kernel((int)variables.v[options.v[0]].num, (int)variables.v[options.v[1]].num);
						break;
					case 20:
						printf("generate_gaussian_blur_kernel\n");
						images.list[outputimg] = generate_gaussian_blur_kernel((int)variables.v[options.v[0]].num, (int)variables.v[options.v[1]].num, variables.v[options.v[2]].num);
						break;
					case 21:
						printf("box_sample\n");
						images.list[outputimg] = box_sample(images.list[variables.v[options.v[0]].img], variables.v[options.v[1]].num);
						break;
					case 22:
						printf("bilinear_interpolation\n");
						images.list[outputimg] = bilinear_interpolation(images.list[variables.v[options.v[0]].img], variables.v[options.v[1]].num);
						break;
					case 23:
						printf("sobel_accumulator_func\n");
////arraytopvalarray(variables, options.v[0]), arraytopvalarray(variables, options.v[1]), (int)variables.v[options.v[2]].num, (int)variables.v[options.v[3]].num);
						break;
					case 24:
						printf("kirsch_accumulator_func\n");
////arraytopvalarray(variables, options.v[0]), arraytopvalarray(variables, options.v[1]), (int)variables.v[options.v[2]].num, (int)variables.v[options.v[3]].num);
						break;
					case 25:
						printf("kirsch_color_accumulator_func\n");
////arraytopvalarray(variables, options.v[0]), arraytopvalarray(variables, options.v[1]), (int)variables.v[options.v[2]].num, (int)variables.v[options.v[3]].num);
						break;
					case 26:
						printf("rotate_90\n");
						images.list[outputimg] = rotate_90(images.list[variables.v[options.v[0]].img]);
						break;
					case 27:
						printf("rotate_180\n");
						images.list[outputimg] = rotate_180(images.list[variables.v[options.v[0]].img]);
						break;
					case 28:
						printf("rotate_270\n");
						images.list[outputimg] = rotate_270(images.list[variables.v[options.v[0]].img]);
						break;
					case 29:
						printf("rotate_skew\n");
						images.list[outputimg] = rotate_skew(images.list[variables.v[options.v[0]].img], variables.v[options.v[1]].num);
						break;
					case 30:
						printf("rotate_rotation_matrix\n");
						images.list[outputimg] = rotate_rotation_matrix(images.list[variables.v[options.v[0]].img], variables.v[options.v[1]].num);
						break;
					case 31:
						printf("clip\n");
////variables.v[options.v[0]].num, variables.v[options.v[1]].num, variables.v[options.v[2]].num);
						break;
					case 32:
						printf("select_subarray\n");
						images.list[outputimg] = select_subarray(arraytoxy(variables, options.v[0]), arraytoxyz_int(variables, options.v[1]), images.list[variables.v[options.v[2]].img]);
						break;
					case 33:
						printf("select_subarray_no_offset\n");
						images.list[outputimg] = select_subarray_no_offset(arraytoxy(variables, options.v[0]), arraytoxyz_int(variables, options.v[1]), images.list[variables.v[options.v[2]].img]);
						break;
					case 34:
						printf("convolute_img\n");
						images.list[outputimg] = convolute_img(images.list[variables.v[options.v[0]].img], images.list[variables.v[options.v[1]].img]);
						break;
					case 35:
						printf("empty_img\n");
						images.list[outputimg] = empty_img((int)variables.v[options.v[0]].num, (int)variables.v[options.v[1]].num, (int)variables.v[options.v[2]].num);
						break;
					case 36:
						printf("empty_colored_img\n");
						images.list[outputimg] = empty_colored_img((int)variables.v[options.v[0]].num, (int)variables.v[options.v[1]].num, (int)variables.v[options.v[2]].num, arraytopvalarray(variables, options.v[3]));
						break;
					case 37:
						printf("regular_plygon\n");
////(int)variables.v[options.v[0]].num, (int)variables.v[options.v[1]].num, variables.v[options.v[2]].num);
						break;
					default:
						return make_error(3, functionstr, linenum, "", 0);
				} //--endswitch--
				printf("end switch\n");
				if(freeimg) {
					//free(imgptr->image);
					//free(imgptr);
				}
				free(options.v);
				free(options.cvi);
			}
		}
	}
	error err;
	err.type = 0;
	err.images = images;
	return err;
}
