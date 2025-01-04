#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <math.h>

int main() {
	char* directory = "./utils/";
	int dirlen = strlen(directory);
	DIR* d;
	struct dirent *dir;
	d = opendir(directory);
	//char** files = malloc(sizeof(char*));
	//int len = 1;
	char* line = NULL;
	size_t linelen = 0;
	ssize_t read;
	FILE* functionsfile = fopen("utils/functions.c", "w");
	int availablefunctions = 0;
	if(d) {
		unsigned int switchstufflen = 28;
		unsigned int switchstufflenprev = 28;
		//char* switchstuff = malloc(switchstufflen);
		char switchstuff[3000];
		unsigned int funcnum = 0;
		char* parserline = NULL;
		size_t parserlinelen = 0;
		ssize_t parserread;
		FILE* parserfile = fopen("utils/parser.c", "r");
		int reachedarray = 0;
		int startoffunc = 0;
		int offsetfunc = 0;
		while((parserread = getline(&parserline, &parserlinelen, parserfile)) != -1) {
			//printf("%s|%d|", parserline, strcmp(parserline, interpstr));
			offsetfunc += parserread;
			if(strcmp(parserline, "error interprate(text t) {\n") == 0) {
				reachedarray = 1;
				fprintf(functionsfile, "%s", parserline);
				startoffunc = offsetfunc;
			} else if(reachedarray == 1 && read > 10) {
				//printf("ggf\n%d |%s|", parserread, &parserline[parserread-10]);
				if(strcmp(&parserline[parserread-10], "//--END--\n") == 0) {
					parserread = getline(&parserline, &parserlinelen, parserfile);
					offsetfunc += parserread;
					break;
				}
			} else {
				fprintf(functionsfile, "%s", parserline);
			}
		}
		//fprintf(functionsfile, "#include \"parser.h\"\n#include \"functions.h\"\n\n                       \n\t//name                             name_len  arg_amount  inputmask\n");
		fprintf(functionsfile, "\n                       \n\t\t//name                              name_len  arg_amount  inputmask\n");
		while ((dir = readdir(d)) != NULL) {
			if(dir->d_name[strlen(dir->d_name)-1] == 'h') {
			//printf("%s\n", dir->d_name);
			//files[len-1] = dir->d_name;
			//len++;
			//files = realloc(files, len*sizeof(char*));
			char file_name[dirlen+strlen(dir->d_name)];
			strcpy(file_name, directory);
			strcat(file_name, dir->d_name);
			printf("%s\n", file_name);
			fprintf(functionsfile, "\t\t//%s\n", file_name);
			FILE* file = fopen(file_name, "r");
			while((read = getline(&line, &linelen, file)) != -1) {
				//printf("%s", line);
				if(read > 12) {
					char start[9];
					memcpy(&start, &line[read-9], 8*sizeof(char));
					start[8] = '\0';
					printf("|%s|", start);
					if(strcmp(start, "--FLAG--") == 0) {
						printf("\"%s\"%d\n%s%d\n", start, linelen, line, read);
						//printf("Retrieved line of length %zu:\n", read);
						//printf("%s", &line[7]);
						int spaces = 0;
						int brackets = 0;
						int funclen;
						int offset;
						unsigned long inputmask = 0;
						int typelen;
						char* type;
						int argpos = 0;
						for(offset = 0; line[offset] != ' '; offset++);

						char* returntype = malloc(offset+1);
						memcpy(returntype, line, offset);
						returntype[offset] = '\0';
						printf("|%s|\n", returntype);
						offset++;
						for(funclen = 0; line[offset+funclen] != '('; funclen++);
						char func[funclen+1];
						memcpy(func, &line[offset], funclen);
						func[funclen] = '\0';
						switchstufflenprev = switchstufflen;
						switchstufflen += floor(log10(funcnum))+(2*funclen)+51;
						//switchstuff = realloc(switchstuff, switchstufflen+9);
						sprintf(switchstuff, "%s\t\t\t\t\tcase %d:\n\t\t\t\t\t\tprintf(\"%s\\n\");\n", switchstuff, funcnum, func);
						switchstufflenprev += funclen+24+floor(log10(funcnum));
						funcnum++;
						if(strcmp(returntype, "My_png") == 0) {
							sprintf(
								switchstuff,
								"%s\t\t\t\t\t\timages.list[outputimg] = %s(",
								switchstuff,
								func
							);
						} else {
							sprintf(switchstuff, "%s////", switchstuff);
						}

						//printf("%.*s\n", funclen, &line[offset]);
						for(int i = offset+funclen+1; i < read-12; i++) {
							spaces++;
							for(typelen = 0; line[i+typelen] != ' '; typelen++);
							type = (char*)malloc(typelen+1);
							memcpy(type, &line[i], typelen);
							type[typelen] = '\0';
							//printf("%s|\n", type);
							if(strcmp(type, "unsigned") == 0) {
								i += typelen+1;
								for(typelen = 0; line[i+typelen] != ' '; typelen++);
								type = (char*)realloc(type, typelen+1);
								memcpy(type, &line[i], typelen);
								type[typelen] = '\0';
								//printf("stuffls: %s|\n", type);
							}
							unsigned int argposlen = floor(log10(argpos/2))+1;
							switchstufflenprev = switchstufflen-1;
							switchstufflen += argposlen+1;
							if(strcmp(type, "My_png") == 0) { //My_png
								//type is 0, i can ignore it
								switchstufflen += 47;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%simages.list[variables.v[options.v[%d]].img], ",
									switchstuff,
									argpos/2
								);
							} else if(strcmp(type, "char*") == 0) { //string
								inputmask += 1 << argpos;
								switchstufflen += 37;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%svariables.v[options.v[%d]].string, ",
									switchstuff,
									argpos/2
								);
							} else if( //num
								strcmp(type, "pval") == 0 ||
								strcmp(type, "float") == 0 ||
								strcmp(type, "double") == 0
							) {
								inputmask += 2 << argpos;
								switchstufflen += 34;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%svariables.v[options.v[%d]].num, ",
									switchstuff,
									argpos/2
								);
							} else if(
								strcmp(type, "int") == 0
							) {
								inputmask += 2 << argpos;
								switchstufflen += 39;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%s(int)variables.v[options.v[%d]].num, ",
									switchstuff,
									argpos/2
								);
							} else if( //array
								strcmp(type, "pval*") == 0
							) {
								inputmask += 3 << argpos;
								switchstufflen += 47;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%sarraytopvalarray(variables, options.v[%d]), ",
									switchstuff,
									argpos/2
								);
							} else if(
								strcmp(type, "xyz_int") == 0
							) {
								inputmask += 3 << argpos;
								switchstufflen += 46;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%sarraytoxyz_int(variables, options.v[%d]), ",
									switchstuff,
									argpos/2
								);
							} else if(
								strcmp(type, "xy_int") == 0
							) {
								inputmask += 3 << argpos;
								switchstufflen += 45;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%sarraytoxy_int(variables, options.v[%d]), ",
									switchstuff,
									argpos/2
								);
							} else if(
								strcmp(type, "xy") == 0
							) {
								inputmask += 3 << argpos;
								switchstufflen += 41;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%sarraytoxy(variables, options.v[%d]), ",
									switchstuff,
									argpos/2
								);
							} else if(
								strcmp(type, "polygon") == 0
							) {
								inputmask += 3 << argpos;
								switchstufflen += 46;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%sarraytopolygon(variables, options.v[%d]), ",
									switchstuff,
									argpos/2
								);
							} else if(
								strcmp(type, "color") == 0
							) {
								inputmask += 3 << argpos;
								switchstufflen += 44;
								//switchstuff = realloc(switchstuff, switchstufflen);
								sprintf(
									switchstuff,
									"%sarraytocolor(variables, options.v[%d]), ",
									switchstuff,
									argpos/2
								);
							} else if(strcmp(type, "My_png*") == 0) { //tmp 'fix'
							} else {return 0;}
							i++;
							if(line[i+typelen] == '(') {
								for(i = i+typelen; i < read-12; i++) {
									switch((int)line[i]) {
										case (int)'(':
											brackets++;
											break;
										case (int)')':
											brackets--;
											break;
									}
									if(brackets == 0) break;
								}
								i++;
							} else {
								for(i = i+typelen; line[i] != ' '; i++);
							}
							if(line[i+1] == ';') break;
							argpos += 2;
						}
						switchstuff[strlen(switchstuff)-2] = '\0';
						sprintf(switchstuff, "%s);\n\t\t\t\t\t\tbreak;\n", switchstuff);
						/*for(int i = offset+funclen+1; i < read-12; i++) {
							if(line[i] == ' ' && brackets < 2) {
								i++;
								spaces++;
								for(typelen = 0; line[i+typelen] != ' '; typelen++) printf("%c", line[i+typelen]);
								printf("\n");
								type = malloc(typelen+1);
								memcpy(&type, &line[i], typelen);
								printf("\n%s\n", type);
								if(strcmp(type, "unsigned") == 0) {
									i += typelen;
									for(typelen == 0; line[i+typelen] != ' '; typelen++) printf("%c", line[i+typelen]);
									type = malloc(typelen+1);
									memcpy(&type, &line[i+1], typelen);
									printf("%s\n", type);
								}
								*//*if(strcmp(type, "My_png") == 0) {
								} else if(strcmp(type, "char*") == 0) { //string
								} else if(strcmp(type, "pval") == 0) { //num
								} else if(strcmp(type, "int") == 0) { //num
								} else if(strcmp(type, "float") == 0) { //num
								} else if(strcmp(type, "double") == 0) { //num
								} else if(strcmp(type, "pval*") == 0) { //array
								} else if(strcmp(type, "xyz_int") == 0) { //array
								} else if(strcmp(type, "xy_int") == 0) { //array
								} else if(strcmp(type, "xy") == 0) { //array
								}*//*
							} else if(line[i] == '(') {
								brackets++;
							} else if(line[i] == ')') {
								brackets--;
							}
						}*/
						//make cases

						printf("%s %d %u\n", func, spaces, inputmask);
						fprintf(functionsfile, "\t\t{\"%s\",%*s%d,%*.s%d,          %u, 1, 0},\n", func, 32-strlen(func), "", funclen, 8-(int)log10(funclen), "", spaces, inputmask);
						availablefunctions++;
					}
				}
			}
			fclose(file);
			}
		}
		closedir(d);
		fseek(functionsfile, -2, SEEK_CUR);
		fprintf(functionsfile, "\n\t}; //--END--");
		fprintf(functionsfile, "\n\tint functionslen = %d;\n", availablefunctions);
		fseek(parserfile, offsetfunc, 0);
		short reachedswitch = 0;
		while((parserread = getline(&parserline, &parserlinelen, parserfile)) != -1) {
			//printf("|%s|\n", &parserline[parserread-16]);
			if(strcmp(&parserline[parserread-16], "--startswitch--\n") == 0) {
				fprintf(functionsfile, "%s%s\t\t\t\t\tdefault:\n\t\t\t\t\t\treturn make_error(3, functionstr, linenum, \"\", 0);\n", parserline, switchstuff);
				reachedswitch = 1;
			} else if(reachedswitch) {
				printf("|%s|\n", &parserline[parserread-14]);
				if(strcmp(&parserline[parserread-14], "--endswitch--\n") == 0) {
					fprintf(functionsfile, "%s", parserline);
					reachedswitch = 0;
				}
			} else {
				fprintf(functionsfile, "%s", parserline);
			}
		}
		fseek(functionsfile, startoffunc, 0);
		fprintf(functionsfile, "\tfunc functions[%d] = {", availablefunctions);
		fclose(parserfile);
		printf("%s", switchstuff);
		//len--;
		//printf(switchstuff);
	}
	fclose(functionsfile);
	//FILE* functionsheaderfile = fopen("utils/functions.h", "w");
	//fprintf(functionsheaderfile, "#ifndef FUNCTIONS_H\n#define FUNCTIONS_H\n\nextern func* functions[%d];\n\nextern int functionslen;\n\n#endif", availablefunctions);
	return 0;
}
