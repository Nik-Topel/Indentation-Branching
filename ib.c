/*                                * 
 *   ib (indentation branching)   * 
 *                                */

/* todo:
 *   -experiment with preprocessing before parsing
 */


#define VERSION "0.12"

#define CONT_MAX 256
#define FILE_MAX 256

#include <limits.h>

#ifndef LINE_MAX
	#define LINE_MAX 2048
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>


#define noreturn _Noreturn

typedef unsigned short psize;

enum ansi_color
{
	blank   = 0,
	red     = 31,
	green   = 32,
	yellow  = 33,
	blue    = 34,
	magenta = 35,
	cyan    = 36,
	white   = 37
};

typedef enum ansi_color color;

struct line_type
{
	psize comment;
	psize tabs;
	char  str[LINE_MAX];
};

typedef struct line_type line_t;

enum filetype
{
	norm,
	c
};

typedef enum filetype type;

struct parser_context
{
	const type ftype;
	bool       mcom;
	psize      tbranchc;
	psize      tbranchs[CONT_MAX];
	psize      nbranchc;
	psize      nbranchs[CONT_MAX];
	psize      ctermc;
	psize      cterms[CONT_MAX];
	psize      bmsgsc;
	psize      bmsgst[CONT_MAX];
	psize      gloc;
	char       bmsgss[CONT_MAX][LINE_MAX];
	line_t     line;
	line_t     l_line;
};

typedef struct parser_context context;

enum arg_mode
{
	noarg,
	nothing,
	space,
	soutput,
	scomp,
	sflags
};

typedef enum arg_mode amode;


psize spaces          = 0;
bool  to_stdout       = false;
bool  integrate       = false;
char  *overwrite_out  = NULL;
char  *overwrite_comp = NULL;
char  *overwrite_flag = NULL;


static void set_color(const color c)
{
	printf("\033[%dm", c);
}

static void msg_out(const size_t num, ...)
{
	va_list messages;
	va_start(messages, num);
	
	size_t x = 0;
	while (x++ < num)
	{
		printf("%s%s", va_arg(messages, const char *), x != num ? ": " : "\n");
	}
	
	va_end(messages);
}

static noreturn void error(const char *msg, const int code, const psize loc)
{
	set_color(red);
	
	if (!loc)
	{
		msg_out(2, "error", msg);
	}
	else
	{
		char locs[16];
		sprintf(locs, "%d", loc);
		msg_out(4, "error", msg, "at line", locs);
	};
	
	set_color(blank);
	exit(code);
}

static void warn(const char *typ, const char *msg, const psize loc)
{
	set_color(magenta);
	
	if (!loc)
	{
		msg_out(3, "warning", typ, msg);
	}
	else
	{
		char locs[16];
		sprintf(locs, "%d", loc);
		msg_out(5, "warning", typ, msg, "at line", locs);
	};
	
	set_color(blank);
}

static void transfere_line(line_t *line, line_t *input_line)
{
	strcpy(line->str, input_line->str);
	
	line->comment = input_line->comment;
	line->tabs    = input_line->tabs;
}

static psize find_valid(const char *line, const char *tok)
{
	char *ptf = strchr(line, '"');
	char *ptr = strstr(line, tok);
	bool open = true;
	
	while (ptf && !(open && ptf > ptr))
	{
		if (open)
		{
			ptr = strstr(ptr + 1, tok);
		}
		ptf  = strchr(ptf + 1, '"');
		
		open = !open;
	}
	
	if (!ptr)
	{
		return strlen(line);
	}
	
	return (psize) (ptr - line);
}

static psize find_end(const char *line, bool *mcom)
{
	psize sline = find_valid(line, "//");
	psize mline = find_valid(line, "/*");
	psize cline = find_valid(line, "*/");
	
	if (!*mcom && mline < sline)
	{
		*mcom = true;
	}
	
	if (*mcom)
	{
		if (cline < sline && cline < mline)
		{
			*mcom = false;
		}
		return 0;
	}
	
	return sline < mline ? sline : mline;
}

static psize get_tabs(const char *line)
{
	return spaces == 0 ? strspn(line, "\t") : strspn(line, " ") / spaces;
}

static const char *minp(const char *x, const char *y)
{
	return x > y ? y : x;
}

static psize get_spaces(const psize tab)
{
	return spaces == 0 ? tab : tab * spaces;
}

static void strip_line(const char *in, char *str)
{
	strcpy(str, in + get_tabs(in));
	
	if (str[strlen(str) - 1] == '\n')
	{
		str[strlen(str) - 1] = '\0';
	}
}

static bool get_line(line_t *line, FILE *file, bool *mcom)
{
	if (!fgets(line->str, LINE_MAX, file))
	{
		line->tabs = 0;
		return false;
	}
	
	psize size = strlen(line->str);
	
	while (line->str[size - 2] == '\\')
	{
		char apnd[LINE_MAX];
		if (!fgets(apnd, LINE_MAX, file))
		{
			break;
		}
		size           += strlen(apnd);
		strcat(line->str, apnd);
	}
	
	line->comment = find_end(line->str, mcom);
	line->tabs    = get_tabs(line->str);
	
	return true;
}

static bool check_word(const char *line, const char *word, const char *stop)
{
	char line_cpy[LINE_MAX];
	strcpy(line_cpy, line);
	
	char *save = NULL;
	char *tok  = strtok_r(line_cpy, " ", &save);
	
	while (tok && (!stop ||tok < stop))
	{
		if (!strcmp(tok, word))
		{
			return true;
		}
		
		tok = strtok_r(NULL, word, &save);
	}
	
	return false;
}

static void terminate(line_t *line, const char tchar, context *cont)
{
	if (line->comment <= get_spaces(line->tabs) + 1)
	{
		return;
	}
	
	if (line->str[line->comment - 2] == ';')
	{
		char log[LINE_MAX];
		strip_line(line->str, log);
		warn("line is already terminated ", log, cont->gloc - 1);
	}
	
	memmove(line->str + line->comment, line->str + line->comment - 1, strlen(line->str + line->comment - 2));
	line->str[line->comment - 1] = tchar;
}

static void brackinate(char **outp, const psize tabs, const char *br)
{
	char *out = *outp;
	psize t = 0;
	
	while(!spaces && t < tabs)
	{
		out[t] = '\t';
		++t;
	}
	
	while(spaces && t < get_spaces(tabs))
	{
		out[t] = ' ';
		++t;
	}
	
	out[t] = '\0';
	
	strcat(out, br);
	
	strcat(out, "\n");
	
	*outp += strlen(out);
}

static void branch_check(char *out, context *cont)
{
	psize       *tabs = &cont->l_line.tabs;
	const psize tar   = cont->line.tabs;
	const char  dir   = tar < *tabs ? -1 : (*tabs < tar ? 1 : 0);
	const psize dec   = dir == 1 ? 1 : 0;
	
	while (tar != *tabs)
	{
		*tabs += (psize) dir;
		
		if (cont->bmsgsc && cont->bmsgst[cont->bmsgsc - 1] == *tabs - dec)
		{
			if (dir == -1)
			{
				--cont->bmsgsc;
				const char *line = cont->line.str;
				if (strncmp(line + get_spaces(*tabs), "#elif", 5) && \
				    strncmp(line + get_spaces(*tabs), "#elifdef", 8) && \
				    strncmp(line + get_spaces(*tabs), "#elifndef", 9) && \
				    strncmp(line + get_spaces(*tabs), "#else", 5))
				{
					brackinate(&out, *tabs - dec, cont->bmsgss[cont->bmsgsc]);
				}
			}
			continue;
		}
		
		if (dir == -1)
		{
			if (cont->ctermc && cont->cterms[cont->ctermc - 1] == *tabs)
			{
				--cont->ctermc;
			}
			if (cont->tbranchc && cont->tbranchs[cont->tbranchc - 1] == *tabs)
			{
				brackinate(&out, *tabs - dec, "};");
				--cont->tbranchc;
				continue;
			}
		}
		
		if (cont->nbranchc && cont->nbranchs[cont->nbranchc - 1] == *tabs - dec)
		{
			if (dir == -1)
			{
				--cont->nbranchc;
			}
			continue;
		}
		
		brackinate(&out, *tabs - dec, dir == 1 ? "{" : "}");
	}
}

static void term_check(context *cont)
{
	line_t *line = &cont->line;
	line_t *l_line = &cont->l_line;
	if (cont->ftype == c && l_line->str[get_spaces(l_line->tabs)] == '#')
	{
		if (line->tabs > l_line->tabs)
		{
			strcpy(cont->bmsgss[cont->bmsgsc], "#endif");
			cont->bmsgst[cont->bmsgsc++] = l_line->tabs;
		}
		
		return;
	}
	if (line->tabs <= l_line->tabs)
	{
		if (cont->ctermc && cont->cterms[cont->ctermc - 1] == l_line->tabs - 1)
		{
			if(line->tabs == l_line->tabs)
			{
				terminate(l_line, ',', cont);
			}
			
			return;
		}
		
		terminate(l_line, ';', cont);
		
	}
	else if (!strchr(l_line->str, '(') && l_line->comment > get_spaces(l_line->tabs) + 1 && \
	         !check_word(l_line->str, "else", l_line->str + l_line->comment) && \
	         !check_word(l_line->str, "do"  , l_line->str + l_line->comment))
	{
		if (l_line->str[l_line->comment - 2] == '=' || \
		    check_word(l_line->str, "enum", minp(strchr(l_line->str, '('), l_line->str + l_line->comment)))
		{
			cont->cterms[cont->ctermc++] = l_line->tabs;
		}
		
		if (l_line->str[l_line->comment - 2] == ':')
		{
			cont->nbranchs[cont->nbranchc++] = l_line->tabs;
			return;
		}
		
		cont->tbranchs[cont->tbranchc++] = l_line->tabs;
	}
}

static void parser(FILE *out, FILE *src, const type ftype)
{
	context cont =
	{
		.ftype    = ftype,
		.tbranchc = 0,
		.nbranchc = 0,
		.ctermc   = 0,
		.bmsgsc   = 0,
		.mcom     = 0,
		.gloc     = 1
	};
	line_t *line = &cont.line;
	line_t *l_line = &cont.l_line;
	
	get_line(l_line, src, &cont.mcom);
	while (true)
	{
		const bool cond = get_line(line, src, &cont.mcom);
		++cont.gloc;
		
		if (line->comment != 0)
		{
			term_check(&cont);
		}
		
		fputs(l_line->str, out);
		
		if (line->comment != 0)
		{
			char branch_line[LINE_MAX];
			branch_line[0] = '\0';
			branch_check(branch_line, &cont);
			if (branch_line[0] != '\0')
			{
				fputs(branch_line, out);
			}
		}
		
		if (!cond)
		{
			break;
		}
		
		transfere_line(l_line, line);
	}
}

static type modeset(const char *path)
{
	const char *dot = strrchr(path, '.');
	
	if (dot)
	{
		if (!strcmp(dot, ".c")   || !strcmp(dot, ".h"))
		{
			return c;
		}
		if (!strcmp(dot, ".cpp") || !strcmp(dot, ".hpp"))
		{
			return c;
		}
	}
	
	warn("unable to detect filetype, disabling all language specific rules", path, 0);
	return norm;
}

static bool vexec(const char *path)
{
	FILE *p = popen(path, "r");
	if (!p) return false;
	
	char buff[LINE_MAX];
	while (fgets(buff, sizeof(LINE_MAX), p))
	{
		printf("%s", buff);
	}
	pclose(p);
	return true;
}

static bool pre_file(char *path, const type ftype)
{
	switch (ftype)
	{
		case c:
			return true;
			if (!spaces)
			{
				spaces = 1;
			}
		case norm:
			
		default:
			return false;
	}
}

static bool comp_file(char *path, const type ftype)
{
	switch (ftype)
	{
		case c:
			char out_path[255];
			strcat(out_path, "gcc -o ");
			strncat(out_path, path, strchr(path, '.') - path);
			strcat(out_path, " ");
			strcat(out_path, path);
			
			if (overwrite_flag)
			{
				strcat(out_path, " ");
				strcat(out_path, overwrite_flag);
			}
			
			char rm_path[255] = "rm ";
			strcat(rm_path, path);
			
			bool ret = vexec(out_path);
			
			ret = vexec(rm_path);
			
			return ret ;
		case norm:
			
		default:
			return false;
	}
}

enum pars_mode
{
	pre,
	imr,
	bodge
};

static void load_file(char *path)
{
	if (integrate)
	{
		if (!pre_file(path, c))
		{
			error("failed to preprocess input", 1, 0);
		}
	}
	
	FILE *out, *src = fopen(path, "r");
	
	if (!src)
	{
		error("failed to open source", 1, 0);
	}
	
	if (to_stdout)
	{
		out = stdout;
	}
	else if (overwrite_out)
	{
		out = fopen(overwrite_out, "wa");
	}
	else
	{
		char out_path[255];
		
		if (strlen(path) < 4 || strcmp(path + strlen(path) - 3, ".ib"))
		{
			warn("defined input is not a ib file", path, 0);
			strcpy(out_path, path);
			strcat(out_path, ".unib");
		}
		else
		{
			strncpy(out_path, path, strrchr(path,'.') - path);
			path[strrchr(path, '.') - path] = '\0';
		};
		
		out = fopen(path, "w");
	};
	
	if (!out)
	{
		error("failed to open destination", 1, 0);
	}
	
	parser(out, src, modeset(path));
	
	fclose(src);
	if (!to_stdout)
	{
		fclose(out);
	}
	
	if (!integrate)
	{
		return;
	}
	
	if (!comp_file(path, modeset(path)))
	{
		error("failed to postprocess output", 1, 0);
	}
}

static noreturn void help(void)
{
	puts("Usage ib: [FILE] ...\n\n" \
	     "ib is a transpiler for languages which are not line based\n" \
	     "\n" \
	     " -h --help      -> print this page\n" \
	     " -v --version   -> show current version\n" \
	     " -o --output    -> overwrite output path for *ALL* defined ib files\n" \
	     " -s --spaces    -> use defined amount of spaces as indentation\n" \
	     " -t --tabs      -> turns spaces mode off *again*\n" \
	     " -i --integrate -> enable pre/post processing\n" \
	     " -c --compiler  -> manually set compiler\n" \
	     " -f --flags     -> add compiler flags\n" \
	     " -S --stdout    -> output to stdout instead of file");
	
	exit(0);
}

static noreturn void version(void)
{
	fputs("Ib version "VERSION"\n", stdout);
	
	exit(0);
}

static amode long_arg_parser(const char *arg)
{
	if (!strcmp("version", arg))
	{
		version();
	}
	
	if (!strcmp("help", arg))
	{
		help();
	}
	
	if (!strcmp("tab", arg))
	{
		spaces = 0;
		return nothing;
	}
	
	if (!strcmp("spaces", arg))
	{
		return space;
	}
	
	if (!strcmp("stdout", arg))
	{
		to_stdout = true;
		return nothing;
	}
	
	if (!strcmp("output", arg))
	{
		return soutput;
	}
	
	if (!strcmp("compiler", arg))
	{
		return scomp;
	}
	
	if (!strcmp("flags", arg))
	{
		return sflags;
	}
	
	warn("invalid option", arg, 0);
	return nothing;
}

static amode short_arg_parser(const char *arg)
{
	psize i = 0 ;
	while(arg[i])
	{
		switch (arg[i])
		{
			case 'h':
				help();
			
			case 'V':
				version();
			
			case 'o':
				return soutput;
			
			case 's':
				return space;
			
			case 'S':
				to_stdout = true;
				return nothing;
			
			case 'c':
				return scomp;
			
			case 'f':
				return sflags;
			
			case 'i':
				integrate = true;
				return nothing;
			
			default :
				char invalid[1];
				invalid[0] = arg[i];
				
				warn("invalid option", invalid, 0);
				
				return nothing;
		}
		i++;
	}
	return nothing;
}

static amode arg_parser(char *arg, const amode last)
{
	if (arg[0] != '-' || strlen(arg) < 2 || last == sflags)
	{
		switch (last)
		{
			case space:
				spaces         = atoi(arg);
				return nothing;
			case soutput:
				overwrite_out  = arg;
				return nothing;
			case scomp:
				overwrite_comp = arg;
				return nothing;
			case sflags:
				overwrite_flag = arg;
				return nothing;
			case nothing:;
			case noarg:
				return noarg;
		}
	}
	
	if (arg[1] == '-')
	{
		return long_arg_parser(arg + 2);
	}
	
	return short_arg_parser(arg + 1);
}

int main(const int argc, char **argv)
{
	if (argc < 2)
	{
		help();
	}
	
	char  *paths[FILE_MAX];
	psize pathc    = 0;
	amode argument = nothing;
	
	while (*++argv)
	{
		argument = arg_parser(*argv, argument);
		
		if (argument == noarg)
		{
			if (pathc == FILE_MAX)
			{
				error("too many files", 1, 0);
			}
			
			paths[pathc++] = *argv;
		}
	}
	
	while (pathc)
	{
		load_file(paths[--pathc]);
	}
	
	exit(0);
}
