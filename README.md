# Simple Literate Programming System

Inspired by the [Literate Haskell](https://wiki.haskell.org/Literate_programming).

## Quick Start

The idea is to generate the executable program and the documentation from the same file:

```console
$ make
$ ./lit -input bubble.tex > bubble.c
$ cc -c bubble.c
$ pdflatex bubble.tex
```

## How It Works?

The program just iterates through the entire file line by line and comments out everything that is not inside of the "code block" so the final result can be passed to a compiler or an interpreter.

The "code block" markers and the comment style are customizable. See `./lit -help` for more information.
