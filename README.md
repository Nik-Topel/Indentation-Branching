# Indentation-Branching

a parser for c for getting rid of curlybraces and semicolons by parsing the identation plus some added language specifics
this allows you to write

~~~
#include <stdio.h>

int main()
  printf("hello world")
  return 0
~~~

and parse it into

~~~
#include <stdio.h>

int main()
{
  printf("hello world");
  return 0;
 }
 ~~~
 
 to make it as usable as possible we also added exception for constructs like enums and structs

its written in its own input althought I provide a bootstrap c file

# Initial Build
~~~
(sudo) make bootstrap
~~~
