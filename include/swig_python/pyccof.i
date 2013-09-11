/* File:pyccof.i */

%module pyccof 

//Make pyccof_wrap.c include this header
%{
#define SWIG_FILE_WITH_INIT
#include "glib.h" 
#include "../cc_of_lib.h"
%}

//Make swig look into specific stuff in this header

%ignore g_direct_hash;

%include "../cc_of_lib.h"




