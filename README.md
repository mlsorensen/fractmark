Introducing FractMark, a side project that I've been working on that started out as a programming exercise.  It's a fractal generator, specifically for the Mandelbrot set.  I eventually decided to use it as a primer for threading applications, and then began using it to benchmark my systems.  It's nice as a benchmark in that it is simple, scales well, and is entirely CPU-bound. It makes good use of any and all CPU resources you've got, provided you select an appropriate number of threads. It provides quick results too; running it for 30 seconds will give you roughly the same scores as you'd get over a 30 minute or 30 hour run. I suppose it could also be used for basic burn-in testing, it certainly gets the processors warmed up. Last, it creates stunning images at any resolution you choose.

It makes use of the GD graphics library, so you'll need to install the library, usually available with any Linux distribution (e.g.  libgd2-xpm-dev for Ubuntu). On Linux, I compile it like so:

gcc -o fractmark fractmark.c -lpthread -lgd -O3

The O3 gives roughly a 3x execution speedup in my tests, most likely due to auto SSE optimizations. This is a pretty math constrained application.

I have also successfully compiled and run it under Windows 7 x64, using MinGW and the GD windows libraries. This can be downloaded as a package, unzipped to your desktop, a USB thumb drive, or wherever and run in a cmd shell as-is.

USAGE

[ms@server ~]# ./fractmark -h

FractMark 0.2 (02/07/2011) by Marcus Sorensen

Usage: fractmark [OPTION]...
With no options, program runs iterations, prints out results, and ends

Defaults used are:
  save image:                        NONE
  number of threads:                 8
  limit of iterations per pixel:     20000
  width in pixels:                   7680
  height in pixels:                  2160
  coordinates (Max X, Min X, Min Y): -0.357000, -0.431000, -0.613000
  color bias (r, g, b):              23, 161, 173

OPTIONS:
  -o FILENAME   Save image as FILENAME(.png default, or specify .png or .jpg in                         name)
  -t INTEGER    Number of worker threads to spawn
  -l INTEGER    Limit of iterations at which to give up and color pixel black
  -W INTEGER    Width of field in pixels
  -H INTEGER    Height of field in pixels
  -X FLOAT      Decimal number, maximum X coordinate of field
  -x FLOAT      Decimal number, minimum X coordinate of field
  -y FLOAT      Decimal number, minimum Y coordinate of field.
                  Note: Max Y is auto-calcualted based on aspect ratio W x H
  -r INTEGER    Red bias of image (0-255)
  -g INTEGER    Green bias of image (0-255)
  -b INTEGER    Blue bias of image (0-255)
  -h            Print this help dialog and exit
MY RESULTS

default settings-

Atom CPU D525 dual core                 FractMark score: 3.46
Core 2 E6600 2.4GHz dual core           FractMark score: 11.32

Pentium E6500 2.93GHz dual core         FractMark score: 13.91 

Xeon X3360 2.83GHz quad core            FractMark score: 27.06
Core i7-2600 3.4GHz quad core           FractMark score: 47.86
Dual AMD Opteron 6274 (32 cores)        FractMark score: 90.57
Dual Xeon E5-2650 2GHz (16/32 core/thr) FractMark score: 135.23
