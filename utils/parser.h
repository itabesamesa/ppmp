#ifndef PARSER_H
#define PARSER_H

typedef struct _error {
	unsigned short type; /*
		0  = no error
		1  = variable has no value
		2  = wrong type for function
		3  = function doesn't exist
		4  = variable doesn't exist
		5  = wrong array length
		6  = var re-/definition within an array
		7  = syntax error with array index: array10]
		8  = wrong number of arguments
		9  = wrong number of results
		10 = function doesn't return any defined type
		*/
	char* function;
	char* variable;
	unsigned int linenum;
	unsigned int arraylen;
	My_png_list images;
} error;

/*typedef struct _array_error {
	short iserror;
	xyz_int;
	xyz;
	xy_int;
	xy;
} array_error;*/

typedef struct _var {
	int type; //-3=error -2=info -1=unkown 0=img 1=string 2=num 3=array 4=defvarimg 5=readvar
	char* name;
	short userdef;
	//char* value;
	char* string;
	double num;
	unsigned int* avi; //array var index
	unsigned int avilen;
	int img;
	unsigned int self;
} var;

typedef struct _localvarlist {
	var* v;
	//unsigned int* udvi; //user defined var index
	//unsigned int udvilen;
	unsigned int vlen;
} localvarlist;

typedef struct _func {
	char* name;
	unsigned int len;
	unsigned int arguments;
	unsigned long inputmask;
	unsigned int outputs;
	unsigned long outputmask;
	//short pseudo;
	//unsigned long arraylengthmask;
} func;

typedef struct _avtypes {
	int type;
	//char* string;
	//double num;
	//int img;
	//int defvarimg;
	//int rw;
	//int readvar;
	unsigned int* v; //var index from localvarlist
	unsigned int vlen;
	unsigned int* cvi; //change var index
	unsigned short uv;
	//unsigned int cvilen; //cvilen must equal function.outputs
	unsigned long cvaction; //0 = read var; 1 = define var; 2 = redefine var
	error err;
} avtypes;

typedef struct _defvarerr {
	unsigned short type;
	unsigned int len;
	int* vi; //var index
	unsigned int uv; //undefined var
	unsigned int* vd; //var defined
	unsigned int vdlen;
} defvarerr;

defvarerr define_vars(char* line, int end, localvarlist* variables);

//void dump_vars(localvarlist variables);

error interprate(text t);

#endif
