# Indentation-Branching

a parser for c allowing you to forego the usage of curlybraces and semicolons by parsing the identation + some added language specifics

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
