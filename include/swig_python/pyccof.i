/*
****************************************************************
**      CodeChix OFconnect - OpenFlow Channel Management Library
**      Copyright CodeChix 2013-2014
**      codechix.org - May the code be with you...
****************************************************************
**
** License:             GPL v2
** Version:             1.0
** Project/Library:     OFconnect, libccof.so
** GLIB License:        GNU LGPL
** Description:    	SWIG Python interface file
** Assumptions:    	python 2.7/2.6, swig 2.0.10
** Testing:             N/A
**
** Main Contact:        deepa.dhurka@gmail.com
** Alt. Contact:        organizers@codechix.org
****************************************************************
*/

%module pyccof 

//Make pyccof_wrap.c include this header
%{
#define SWIG_FILE_WITH_INIT
#include "glib.h"
#include "../cc_of_lib.h"

%}

//Make swig look into specific stuff in this header

%include "../cc_of_lib.h"




