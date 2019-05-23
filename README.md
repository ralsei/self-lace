# Self-lacing Shoe

My final project for Swartzendruber's Digital Electronics class in 2019, built w/ Skillet (https://github.com/killerskyman).
It's a shoe that ties itself.

## Render

![](https://raw.githubusercontent.com/ralsei/self-lace/master/PresStuff/WhinchFullNM(low1).png)

## Licensing

It's under the BSD license with an additional clause stating that you can't use this for PLTW.

## Building

The CAD is made in Inventor, and the PCB is made in EAGLE.
To build the code, you must make the PCB as specified, bootload it, then upload code via FTDI.

Use the script `build` with the following parameters:

`./build main.ino <your TTY here>`
