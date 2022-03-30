# Soundsynth
### Original authors: Daniel Huber and Moray Yesilgüller


## Overview

This project is about ***digital sound synthesis***. It encompasses some music theory, computer science and mathematics.
The actual project is a ***Python3 C-Extension*** that calculates soundwaves of different types. Some additional functions
are added to the module for tinkering with generating music digitally and programmatically. The default output is a **.raw**
file. A terminal audio player that allows to set bitrate, format and sample size manually can play the output. 

## How to use

`make` will generate all the executables for you. You can then use the python module in the current directory or you can
install the module to your path with `make install`. This project has been tested on MacOS and Linux. No guarantees that it works on Windows yet.

## Goal

This project is actually part of an IT assignment but also the product of countless hours of passion and rage.
While this project serves no specific purpose (yet), it was done to get a practical understanding of wave physics
and learn how to write ***C-Extensions*** for ***Python3***.

## License

As stated in the LICENSE file that is located in the same directory as this file, this project is licensed under
the **AGPL-v3.0**.
