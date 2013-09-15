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
** Description:    SWIG Java interface for LibCCOF
** Assumptions:    N/A
** Testing:        N/A
** Authors:        Rupa Dachere
**
*****************************************************
*/

%module cc_of_driver_java

%{
#include "stdint.h"
#include "../cc_of_lib.h"
%}

%include "../cc_of_lib.h"
