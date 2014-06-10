#!/bin/bash

##
#################################################################
##      CodeChix OFconnect - OpenFlow Channel Management Library
##      Copyright CodeChix 2013-2014
##      codechix.org - May the code be with you...
#################################################################
##
## License:             GPL v2
## Version:             1.0
## Project/Library:     OFconnect, libccof.so
## GLIB License:        GNU LGPL
## Description:         Clean up script for python swig interface
## Assumptions:         N/A
## Testing:             N/A
##
## Main Contact:        deepa.dhurka@gmail.com
## Alt. Contact:        organizers@codechix.org
#################################################################
##

# Run as root, of course.
sudo rm -rf pyccof_wrap.c 
sudo rm -rf pyccof.py 
sudo rm -rf pyccof.pyc 
sudo rm -rf build 

echo "Build cleaned up..."

