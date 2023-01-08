# Indentation-Branching

a transpiler for c allowing you to forego the usage of curlybraces, semicolons and in case of enums and struct initialisation lists, commas, by parsing the identation, newlines + some added language specifics

this reduces unneccasary complexity as most of us implicitly use this format while writing code, and only add these symbols because the language requires it

one of the most common complaints about programming in c-style languages is missing semicolons

ib is written in its own input, althought I also provide a transpiled c file

ib itself is approximatly 60% of the length of its verbose c counterpart

### Example

~~~
// example.c.ib

#include <stdio.h>

/*
	multiline comments work sorta
*/

enum dir
	negative
	positive
	neutral

struct point
	enum dir x
	enum dir y

int main()
	struct point pt =
		.x = positive
		.y = negative
    
	if (pt.x = positive && pt.y = negative)
    		printf("unbelievable!! " \\
		       "multiline string!!!\n")
		return 0
	else
		return 1 // comment after termination
~~~

~~~
$ ib example.c.ib
~~~

~~~
// example.c

#include <stdio.h>

/*
	multiline comments work sorta
*/

enum dir {
	negative,
	positive,
	neutral
};

struct point {
 	enum dir x;
 	enum dir y;
};

int main()
{
	struct point pt = {
    		.x = positive,
		.y = negative
	};
    
    	if (pt.x = positive && pt.y = negative)
    	{
		printf("unbelievable!! " \\
		       "multiline string!!!\n");
		return 0;
	}
	else
	{
		return 1; //comment after termination
	}
}
 ~~~

### Initial Build
~~~
(root) $ make bootstrap
~~~

### Build with IB installed

~~~
$ make
$ make test (transpiles itself a second time with the new version,
to ensure that it is functional before bricking your install)
root $ make install
~~~

### Editors

looks best with a smaller tab size, using spaces as tabs is supported by a special flag but not recommended.

displaying tabs and spaces is strongly recommended as missing any indentation will completly break your code

typically the syntax highlighting of the language works if forced

VIM:
~~~
au! BufRead,BufNewFile *.c.ib setfiletype c

set tabstop=4
set shiftwidth=4

set list
set listchars=tab:>\ ,space:.
~~~
