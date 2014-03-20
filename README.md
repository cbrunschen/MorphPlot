# MorphPlot

MorphPlot is mainly a library used to implement a couple of programs. The documentation below is about the "plot" program.

The program assumes that one pixel == one plotter coordinate step. HP's plotters have pretty high resolution – 0.025 mm ... or 1016 coordinates per inch. So that's the resolution of the image that the program expects.

So of you have a PostScript file, for instance, you can use GhostScript to create a suitable PPM file quite easily:

> gs -sDEVICE=png16m -r1016 -sOutputFile=page_%03d.png file.ps

... will render 'file.ps' into a number of raw PNG images, 'page_000.png', 'page_001.png' etc.

The code as set up expects the LIBPNG library to be installed in /usr/local.

You should be able to build the program just by unpacking the tarball, opening the "MorphPlot.xcodeproj" project file using Xcode, and building the 'plot' target from within Xcode.

To use the Makefile, you'd change into the 'MorphPlot' directory that this creates, and type 'make'. If all goes well, this will produce a program, 'plot'.

In either case, it's this 'plot' program that we want to use; copy it to somewhere in your execution path; the examples below presume that you've copied the program to your current working directory.

"plot", again, has little documentation, but I'll summarize here:

plot [ options ] PNG_image_file

Options are:

  -pen pen specification

    The pen specification is of the form key=value[,key=value...]:

    - 'r' sets the pens radius, in pixels

    - 'm' sets the minimum size of feature (in pixels) for which this pen can be used

    - 'c' sets the pen's color, in hex 'cyan-magenta-yellow' form - effectively, the inverse of red-green-blue. For instance, a black pen would be specified with 'c=FFFFFF', a red one with 'c=00FFFF', and so on.

    - 'h' sets the HPGL index - the pen's number to be used in the generated HPGL code

    - 'a' sets the angle against the horizontal axis to be used when hatching (filling the inside of a filled area) with this pen; this allows different pens in different colors to not just draw along, but across each other. The unit here is 'pi-radians' - from -1 (-180 degrees) to 1 (+180 degrees). For pens that you specify, the default angle will be 0.

    - 'p' sets the hatching phase and is an offset in pixels from the origin, for the placement of the hatch lines. Don't worry about this, the default should be fine.

    If you specify no pens, a default set of 8 pens will be used, each with ~`a different ink color and hatch angle. 

    A simple specification of a normal-ish black pen, 0.7mm thick (so with a radius of 0.35mm / 0.025 mm/coord = 14 coordinates), with a hatching angle of about 30 degrees off the horizontal, as HPGL pen 1:

    -pen c=FFFFFF,r=14,a=0.167,h=1


  -rotate l | r | u

    rotates the entire plot Left, Right or Upside-down. This is to handle the fact that in most pen plotters, a Letter or A4 sheet is by default placed horizontally, in 'landscape' orientation; if you want to plot something that was originally in portrait orientation, you'll want to rotate it. 


  -offset offset | horizontalOffsetxverticalOffset

    allows you to add the specified number to each generated coordinate. If you only specify one 'offset' value, it will be applied to both X and Y coordinates; to offset 100 pixels to the right and 200 up, use '-offset 100x200'.


  -steps[=step_filename_prefix]

    will generate bitmaps for each partial step along the way, which give you some information about the progress of the processing and how the program does its thing. Mainly for debugging and education.


  -approx[=approximation_filename]

    makes the program also generate a bitmap of the approximation it reckons it has created of the original image. This handles cases where multiple pens draw over the same pixel by subtracting the colors (i.e., adding the effects of the inks).


  -out[type] outputFilename

    Specifies one output to be generated to the specified file in the specified format:

    - 'HP' or 'HA': HPGL with absolute coordinates; simplest, most explicit and probably most compatible; and the default type if no other type is specified.

    - 'HR: HPGL with relative coordinates; still simple but a bit smaller.

    - 'HE': HP/GL-2 using the 'PE', 'Polyline Encoded', instruction to compress pen movements, using 8-bit characters. Requires HP/GL-2 and a clean 8-bit-per-character connection to the plotter.

    - 'HE7: HP/GL-2 using the 'PE', 'Polyline Encoded', instruction to compress pen movements, but using only 7-bit characters. Requires HP/GL-2, but does not require a clean 8-bits-per-character connection. Compresses slightly less well then 'HE'.

    - 'PS' : generate PostScript with the same pen moves, using the specified pen colors and sizes. note: when one pen draws over another in PostScript, the top one _replaces_ anything underneath; this means that if a yellow line os drawn on top of black, for instance, the result will be yellow, whereas on an actual pen plotter the result will still be black. This is useful mainly for plots with a single pen, for seeing a scalable approximation of the output.

    You can specify multiple outputs. If the outputFilename is '-', the result will be sent to standard output. If no output is specified at all, HPGL with absolute coordinates will be generated to standard output as if '-outHP -' had been specified.


When processing the image, pens are worked on effectively in the order they're encountered on the commend line, with the exception that pens in the same ink color are processed together, in decreasing order of radius.

For each ink color, the program tries to extract the maximum amount that that ink color is present in each pixel; it generates effectively a greyscale image of the 'coverage' of that color within the full-color image, and subtracts that from the remaining image to be processed later.

Then, starting with the widest pen in that color, it determines the outlines it can draw that do not extend outside of the actual shapes; and then generates a hatch pattern to cover the insides of these shapes. anything that was smaller than the width of the pen is left over for thinner pens.

Once it has the paths along which the pen travels, it then uses a simple linear error diffusion to try to approximate different "gray" levels - in an area of less coverage, the pen will be down for some parts and up for other parts of its movement. 

This then proceeds with the next thinner pen in the same ink color, which now only has to cover things not covered by wider pens.

With the thinnest pen in a given ink color, the program then tries to determine what else it can approximate by drawing through the centers of things that are smaller than the width of the pen, but still at least as big as the 'minimum size' specified (which defaults to half the pen's radius); it generates those pen movements and then does the greyscale approximation with those as well. This allows even features that are smaller than any of the pens to be represented in the approximation - which can help make text legible for instance, otherwise thin connecting lines may be lost.

If you have GhostScript installed (easily done using MacPorts), you can easily generate a bit of a demonstration with something like this. To generate a small sample file using GhostScript:

    gs -dNOPAUSE -g1200x500 -r1016 -sDEVICE=png16m -sOutputFile=/tmp/page.png -c "/Times-Bold findfont 24 scalefont setfont 3 10 moveto (Testing) show showpage quit"

To open this file in Preview, double-click it in Finder, or:

    open /tmp/page.png

To then process this file into HPGL, while also generating the 'step' images (all named '/tmp/step_something') and the final approximation (in /tmp/approx.png):

    ./plot -steps=/tmp/step -approx=/tmp/approx.png -pen h=1,c=ffffff,r=7,a=0.1 -outHP=/tmp/out.hgpl -outPS=/tmp/out.ps /tmp/page.png

To open all those generated images in Preview:

    open /tmp/step*png /tmp/approx.png

This should also give a lot of output describing what it's doing; and because we're requesting that it show us the steps, and an approximation, those will be there to look at as well (in /tmp with the command above).
