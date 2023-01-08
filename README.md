# Indentation-Branching

A transpiler for c-style languages allowing you to forego the usage of curlybraces, semicolons, in case of enums and struct initialisation lists, commas and for preprocessor branching "#endif" by parsing the identation, newlines and some added language specifics.

This reduces unnecessary complexity as most of us implicitly use this format while writing code, and only add these symbols because the language requires it.

One of the most common issues in c-style languages is missing semicolons and the endless debate where to put the braces.

IB is written in its own input, it is approximately 60% of the length of its transpiled c counterpart and 90% of the file size.

### Example

```c
// example.c.ib

#if 1
    #include <stdio.h>
#else
    #error "not possible"

/*
    multiline comments work sorta
*/

enum dir
    negative = -1
    positive = 1
    neutral  = 0

struct point
    enum dir x
    enum dir y

int main()
    struct point pt =
        .x = positive
        .y = negative
    
    if (pt.x = positive && pt.y = negative)
        printf("unbelievable!! " \
               "multiline string!!!\n")
        return 0
    else
        return 1 // comment after termination
		
```

```
$ ib example.c.ib --spaces 4 (note: this flag is only required for people who use spaces to indent)
```

```c
// example.c

#if 1
    #include <stdio.h>
#else
    #error "not possible"
#endif

/*
    multiline comments work sorta
*/

enum dir {
    negative = -1,
    positive = 1,
    neutral  = 0
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
        printf("unbelievable!! " \
               "multiline string!!!\n");
        return 0;
    }
    else
    {
        return 1; //comment after termination
    }
}
```

### Initial Build
~~~
(root) $ make bootstrap (compiles provided pretranspiled file and installs it in ${PREFIX}/bin/)
~~~

### Build with IB installed

~~~
$ make

$ make test (transpiles itself to ensure that you are not installing a faulty build)

(root) $ make install
~~~

### Editors

looks best with a smaller tab size, using spaces as tabs is supported by a special flag but not recommended.

displaying tabs and spaces is strongly recommended as any improper indentation will completly break your code

typically the syntax highlighting of the language works if forced

- VIM:
~~~
au! BufRead,BufNewFile *.c.ib setfiletype c

set tabstop=4
set shiftwidth=4

set list
set listchars=tab:>\ ,space:.
~~~
