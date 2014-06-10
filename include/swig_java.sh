#!/bin/bash

##################################################################
##      CodeChix OFconnect - OpenFlow Channel Management Library
##      Copyright CodeChix 2013-2014
##      codechix.org - May the code be with you...
##################################################################
##
## License:        GPL v2
## Version:        1.0
## LibraryName:    OFconnect, libccof.so
## GLIB License:   GNU LGPL
## Description:    This script generates all the necessary
##		   intermediate files/directories for creation
##		   of the JAR file for the CodeChix ONF Driver
##		   API's. 
## Assumptions:	   OS Image = Ubuntu. This script assumes that
##		   "swig" is installed in /usr/bin/swig.  If not,
##		   modify the "run swig" section with the correct path.
## Testing:	   To use the API from your Java test program,
##		   i.e., test.java, import "com.codechix.onf_driver.*"
##		   and then compile with 
##		   "javac -classpath ./onf_driver_swig.jar test.java"
##		   and then run with
##		   "java -classpath .:./onf_driver_swig.jar test"
## Main Contact:   deepa.dhurka@gmail.com
## Alt. Contact:   organizers@codechix.org
##
##################################################################

#get swig, jdk, glib
sudo apt-get install swig
sudo apt-get install openjdk-7-jdk icedtea-7-plugin
sudo apt-get install libglib2.0-dev

# changedir to swig_java
cd swig_java

# run swig
/usr/bin/swig -java -package org.codechix.onf_driver cc_of_driver_java.i

## For some reason we need to hardcode <glib.h> in order to resolve "gboolean"
## compilation errors
# modify cc_of_driver_java__wrap.c with #include <glib.h>

sed -e '/<jni.h>/a\#include <glib.h>' cc_of_driver_java_wrap.c > foo.c
mv foo.c cc_of_driver_java_wrap.c

## Compile the wrapper 
gcc -c -fpic cc_of_driver_java_wrap.c -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include -lglib-2.0 -I/usr/lib/jvm/java-1.7.0-openjdk-i386/include/ -I/usr/lib/jvm/java-1.7.0-openjdk-i386/include/linux

## Compile the java files and generate the package dir structure
javac -d . *.java 

## Create the shared object
gcc -shared cc_of_driver_java_wrap.o -o lib_cc_of_driver.so

## Build the JAR file
jar cf onf_driver_swig.jar org/codechix/onf_driver/*.class

