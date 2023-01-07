/*                                *;
 *   ib (indentation branching)   *;
 *                                */;

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <string.h>

#define MAX_LN 510
#define MAX_CONT 16
#define VERSION "0.1"

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

static noreturn void error(const char *msg, const int code)
{
	printf("\033[36m");
	msg_out(2, "error", msg);
	printf("\033[0m");
	exit(code);
}

static void warn(const char *typ, const char *msg)
{
	printf("\033[33m");
	msg_out(3, "warning", typ, msg);
	printf("\033[0m");
}

static void info(const char *typ, const char *msg)
{
	msg_out(2, typ, msg);
}

struct line_type
{
	size_t comment;
	size_t tabs;
	char   str[MAX_LN];
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
	size_t     tbranchc;
	size_t     tbranchs[MAX_CONT];
	size_t     nbranchc;
	size_t     nbranchs[MAX_CONT];
	size_t     ctermc;
	size_t     cterms[MAX_CONT];
	const type ftype;
};

typedef struct parser_context context;

static void transfere_line(line_t *line, line_t *input_line)
{
	strcpy(line->str, input_line->str);
	line->comment   = input_line->comment;
	line->tabs      = input_line->tabs;
}

static size_t find_end(const char *line)
{
	char *ptfu = strchr(line, '\'');
	char *ptf  = strchr(line, '"');
	char *ptr  = strstr(line, "//");
	bool open  = true;
	while (ptf && !(open && ptf > ptr))
	{
		if (open)
		{
			ptr = strstr(ptr + 1, "//"); // test
		}
		ptf  = strchr(ptf + 1, '"');
		open = !open;
	}
	if (!ptr)
	{
		return strlen(line);
	}
	return (size_t) (ptr - line);
}

static bool get_line(line_t *line, FILE *file)
{
	if (!fgets(line->str, MAX_LN, file))
	{
		line->tabs = 0;
		return false;
	}
	size_t size = strlen(line->str);
	line->tabs = strspn(line->str, "\t");
	while (line->str[size - 2] == '\\')
	{
		char apnd[MAX_LN];
		if (!fgets(apnd, MAX_LN, file))
		{
			break;
		}
		size           += strlen(apnd);
		strcat(line->str, apnd);
	}
	line->comment = find_end(line->str);
	return true;
}

static bool check_word(const char *line, const char *word, const char *stop)
{
	char line_cpy[MAX_LN];
	strcpy(line_cpy, line);
	char *tok = strtok(line_cpy, " ");
	while (tok && (!stop ||tok < stop))
	{
		if (!strcmp(tok, word))
		{
			return true;
		}
		tok = strtok(NULL, word);
	}
	return false;
}

static void terminate(line_t *line, const char tchar)
{
	if (line->comment <= line->tabs + 1)
	{
		return;
	}
	if (line->str[line->comment - 2] == ';')
	{
		warn("line is already terminated ", line->str);
	}
	memmove(line->str + line->comment, line->str + line->comment - 1, strlen(line->str + line->comment - 2));
	line->str[line->comment - 1] = tchar;
}

static void brackinate(FILE *out, const size_t tabs, const char br, context *cont)
{
	size_t t = 0;
	while(t++ < tabs)
	{
		fputc('\t', out);
	}
	fputc(br, out);
	if (br != '{' && cont->tbranchc && cont->tbranchs[cont->tbranchc - 1] == tabs)
	{
		fputc(';', out);
		--cont->tbranchc;
	}
	fputc('\n', out);
}

static void branchcheck(FILE *out, size_t *tabs, const size_t tar, context *cont)
{
	const char dir = tar < *tabs ? -1 : (*tabs < tar ? 1 : 0);
	while (tar != *tabs)
	{
		if (!(cont->nbranchc && cont->nbranchs[cont->nbranchc - 1] == *tabs))
		{
			brackinate(out, dir == -1 ? *tabs - 1 : *tabs, dir == 1 ? '{' : '}', cont);
		}
		else
		{
			--cont->nbranchc;
		}
		*tabs += (size_t) dir;
		if (dir == -1 && (cont->ctermc && cont->cterms[cont->ctermc - 1] == *tabs))
		{
			--cont->ctermc;
		}
	}
}

static void termcheck(line_t *line, line_t *l_line, context *cont)
{
	if (l_line->str[l_line->tabs] == '#')
	{
		return;
	}
	if (line->tabs <= l_line->tabs)
	{
		if (cont->ctermc && cont->cterms[cont->ctermc - 1] == l_line->tabs - 1)
		{
			if(line->tabs == l_line->tabs)
			{
				terminate(l_line, ',');
			}
			return;
		}
		terminate(l_line, ';');
	}
	else if ((!strchr(l_line->str, '(') && l_line->comment > l_line->tabs + 1 && \
	          strncmp("else", l_line->str + l_line->tabs, 4) && \
	          strncmp("do", l_line->str + l_line->tabs, 2)))
	{
		if (l_line->str[l_line->comment - 2] == '=' || \
		    check_word(l_line->str, "enum", strchr(l_line->str, '(')))
		{
			cont->cterms[cont->ctermc++] = l_line->tabs;
		}
		cont->tbranchs[cont->tbranchc++] = l_line->tabs;
	}
	else if (l_line->str[l_line->comment - 2] == ':')
	{
		cont->nbranchs[cont->nbranchc++] = l_line->tabs;
	}
}

static void parser(FILE *out, FILE *src, const type ftype)
{
	context cont =
	{
		.tbranchc = 0,
		.nbranchc = 0,
		.ctermc   = 0,
		.ftype    = ftype
	};
	line_t  line, l_line;
	get_line(&l_line, src);
	while (true)
	{
		const bool cond = get_line(&line, src);
		termcheck(&line, &l_line, &cont);
		fputs(l_line.str, out);
		branchcheck(out, &l_line.tabs, line.tabs, &cont);
		if (!cond)
		{
			break;
		}
		transfere_line(&l_line, &line);
	}
}

static type modeset(const char *path)
{
	const char *dot = strrchr(path, '.');
	if (!dot)
	{
		warn("unable to detect filetype, disabling all language specific rules", path);
		return norm;
	}
	if (!strcmp(dot, ".c") || !strcmp(dot, ".h"))
	{
		return c;
	}
}

static void load_file(char *path)
{
	if (strlen(path) < 3 || strcmp(path + strlen(path) - 3, ".ib"))
	{
		error("defined input is not a ib file", 1);
	}
	FILE *src = fopen(path, "r");
	if (!src)
	{
		error("failed to open source", 1);
	}
	char out_path[255];
	strncpy(out_path, path, strrchr(path,'.') - path);
	path[strrchr(path, '.') - path] = '\0';
	FILE *out = fopen(path, "w");
	if (!out)
	{
		error("failed to open destination", 1);
	}
	info("transpilling", path);
	parser(out, src, modeset(path));
	fclose(src);
	fclose(out);
}

static void help()
{
	puts("Usage ib: [FILE] ...\n" \
	     "ib is a parser for languages which are not line based\n\n" \
	     " -h --help    -> print this page\n" \
	     " -v --version -> show current version");
	exit(0);
}

static void version()
{
	fputs("Ib version "VERSION"\n", stdout);
	exit(0);
}

static bool argument_parser(const char *arg)
{
	if (arg[0] != '-' || strlen(arg) < 2)
	{
		return false;
	}
	switch (arg[1])
	{
		case '-':
		{
			if (!strcmp("version", arg + 2))
			{
				version();
			}
			if (!strcmp("help", arg + 2))
			{
				help();
			}
			warn("invalid option", arg);
			return true;
		};
		case 'h':
		{
			help();
		};
		case 'V':
		{
			version();
		};
		default :
		{
			warn("invalid option", arg);
			return true;
		};
	}
}

int main(const int argc, char **argv)
{
	if (argc < 2)
	{
		error("missing arguments", 1);
	}
	char **paths = malloc(sizeof(char*) * (size_t) argc);
	size_t pathc = 0;
	while (*++argv)
	{
		if (!argument_parser(*argv))
		{
			paths[pathc++] = *argv;
		}
	}
	while (pathc)
	{
		load_file(paths[--pathc]);
	}
	free(paths);
	exit(0);
}
