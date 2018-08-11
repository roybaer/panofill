# panofill

The command line tool `panofill` is a program for automatically completing spherical 360°×180° panorama images that takes the characteristics of this projection into account.

[![](img/test_in.png)![](img/test_out.png)](examples.md)

## Objective

The goal of this project is to provide a platform-independent program suitable for batch processing, which integrates perfectly into the panorama creation workflow.

## Platform

Except for libtiff, getopt and gettext, `panofill` only uses the C++ standard library and should therefore be compilable on any sufficiently powerful machine for which a GNU C++ compiler is available. The use of some other C++ compiler is also thinkable.

## Current mode of operation

The program operates on a Gaussian pyramid adapted to the spherical projection. These adaptations are a variable blur filter width that increases in the vicinity of the poles and additionally, the behavior at the edges of the image.

- Read TIFF file
- Create floating point representation of the image _(layer 0 of the Gaussian pyramid)_
  - Create next Gaussian pyramid layer; multiply alpha channel with a fixed value, clamping at upper limit
  - Increase recursion depth if the topmost Gaussian pyramid layer still contains transparent pixels
  - Blend the two topmost layers and delete the topmost layer
- Create 32 bit representation _(red, green, blue and alpha with 8 bits each)_ from layer 0
- Save result to TIFF file

## Prospect

Future extensions might include technology from the field of texture synthesis. It is conceivable to use e.g. stochastic texture synthesis to simulate random or random-like effects such as sensor noise, asphalt, snow, clouds, grass, etc.
