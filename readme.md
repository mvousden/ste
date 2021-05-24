A Simple String-Templating Tool
===

A small moustache-inspired templating tool, ala Jinja, but for C89. Nice and
(somewhat) portable.

What does it do?
---

Loads a file and performs substitutions based on moustache (`{{` and `}}`
presence), and writes the file somewhere else.

Here's a POSIX example: lets say you have a file at `input`, with contents:

```
<SomeTag>{{f:./b/c.txt}}</SomeTag>
```

and another file `b/c.txt` (i.e. a file named `c.txt` in a directory named
`b`), with contents:

```
Hello World!
This is a multiline file
{{f:./d.c}}
```

and yet another file `b/d.c`, which contains (with a newline at EOF):

```
Quack Quack jibber jibber
```

If `a.xml` is read by this templating library, it writes the following
somewhere specified by the user:

```
<SomeTag>Hello World!
This is a multiline file
Quack quack jibber jibber
</SomeTag>
```

...with no newline at EOF.

Note that:

 - The `f:` prefix in the moustache tells the templater that this moustache
   refers to a file.

 - Moustaches can't be escaped. I don't want to make it too easy.

 - Relative paths are relative to the file they are used in. They are not
   necessarily relative to the primary input file (see what happened to `b/d.c`
   in the example above).

 - Errors are printed to stderr, and a non-zero value is returned by the
   templater if an error is enountered.

Pretty simple, no?

How do I use it?
---

You'll need a C89 compiler to compile this. There's a makefile, which builds an
executable and a shared object (which can be used with the header), if that's
your jam.

If you're using the executable, the first argument must be the path to your
input file, and the second argument must be the path to your output file.

Compatibility and Language
---

C89, but you can compile this library into a shared object for use in your
language of choice.

The templating mechanism uses POSIX' basename method to resolve filenames, so
if you're on Windows, you'll need to code this in yourself, because I have no
idea.


Gotcha: Nesting and Recursion
---

If you've got a particularly nasty case that has many degrees of nesting, you
may fall foul of the OS' limit on the number of file descriptors a process can
open at a time. The example in `examples/infinite` demonstrates this, by
printing the following to `stderr`:

```
Error opening nested file 'examples/infinite/input': Too many open files.
```

To view the limit in most linuxes, `ulimit -n` is your friend.

Don't forget to remove the large file created by this example after you've
finished!
