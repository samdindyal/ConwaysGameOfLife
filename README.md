###Conway's Game of Life (Clone Call Implementation)

This is an implementation of Conway's Game of Life in C, using Linux's "clone" call, that I created for my Intro to Operating Systems course for school. It takes two command line arguments, the first one being the directory of the input file and the second being the amount of threads to use. The syntax for running it is as follows:

```bash
./A1 file 4
```

Where the directory of the input file is represented by "file" and the number of threads going to be created is 4. It can also be run with a single command line argument, being the directory of the input file and the syntax for that is as follows:

```bash
./A1 file
```

Where "file" is the directory of input file. If executed this way, it will run with a single thread.

I have created some test files and have put them in the "res" folder in the root directory of this repository. Feel free to try them out.
