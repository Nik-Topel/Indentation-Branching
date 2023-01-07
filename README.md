# Indentation-Branching

a transpiler for c allowing you to forego the usage of curlybraces, semicolons and in case of enums and struct initialisation lists, commas, by parsing the identation, newlines + some added language specifics

this allows you to write

~~~
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

and transpile it into

~~~
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

its written in its own input, althought I do provide a bootstrap c file

### Initial Build
~~~
(sudo) make bootstrap
~~~

### Build with IB installed

~~~
make
make test (transpiles itself a second time with the new version to ensure that it is functional before bricking your install)
(sudo) make install
~~~
