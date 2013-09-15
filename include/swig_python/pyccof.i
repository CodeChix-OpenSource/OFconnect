/*
*****************************************************
**      CodeChix ONF Driver (LibCCOF)
**      codechix.org - May the code be with you...
**              Sept. 15, 2013
*****************************************************
**
** License:        Apache 2.0 (ONF requirement)
** Version:        0.0
** LibraryName:    LibCCOF
** GLIB License:   GNU LGPL
** Description:    SWIG Python interface file for LibCCOF
** Assumptions:    python 2.7/2.6, swig 2.0.10
** Testing:        N/A
** Authors:        Swapna Iyer
**
*****************************************************
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




