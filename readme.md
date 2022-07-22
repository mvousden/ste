[![CircleCI](https://circleci.com/gh/mvousden/ste.svg?style=shield)](https://app.circleci.com/pipelines/github/mvousden/ste)
![License](https://img.shields.io/badge/License-BSD%202--Clause-brightgreen.svg)

A Simple Templating Engine
===

A small moustache-inspired templating tool, ala Jinja, but for C89. Nice and
(somewhat) portable.

What does it do?
---

Loads a (series of) ASCII files, and performs substitutions based on moustache
(`{{` and `}}` presence), and writes the file somewhere else.

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
Quack {{v:anothernoise}}
```

If `input` is read by this templating library, by running:

```
./ste input output anothernoise=quack
```

it writes the following to `output`:

```
<SomeTag>Hello World!
This is a multiline file
Quack quack
</SomeTag>
```

...with no newline at EOF.

Note that:

 - The `f:` prefix in the moustache tells ste that this moustache refers to a
   file somewhere, which can be opened and read.

 - The `v:` prefix in the moustache tells ste that this moustache refers to a
   value passed to it by the user.

 - Moustaches can't be escaped, but if your moustache doesn't have a code
   (e.g. `f:`), they'll be ignored anyway.

 - Relative paths are relative to the file they are used in. They are not
   necessarily relative to the primary input file (see what happened to `b/d.c`
   in the example above).

 - Errors are printed to stderr, and a non-zero value is returned by ste if an
   error is encountered.

Pretty simple, no?

How do I use it?
---

You'll need a C89 compiler to compile this. There's a makefile, which builds an
executable and a shared object (which can be used with the header), if that's
your jam.

If you're using the executable, the first argument must be the path to your
input file, the second argument must be the path to your output file, and
arguments after this must be `handle=value` for each value you want to define.

Compatibility and Language
---

C89, but you can compile this library into a shared object for use in your
language of choice.

Note that this uses POSIX' basename method to resolve filenames, so if you're
on Windows, you'll need to code this in yourself, because I have no idea.

The included quick-and-dirty test script uses `valgrind`'s `memcheck` tool, so
you'll probably want to install that if you want to run the tests.

String Helpers
---

The string helpers source defines some methods otherwise available in libbsd
used here, which I redefine here for portability and convenience. I can't
guarantee they'll function identically, but there is a test program!

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
