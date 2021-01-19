# "@(#) $Id: ACMM:DramaErs/README.txt,v 3.19 09-Dec-2020 17:17:02+11 ks $"

DramaErs is Drama's error porting system libray.  Its allows structured error
reporting from DRAMA software, and can be built in a standalone mode.

When building standalone, you must have a copy of Ers_Err.h from the DRAMA build.

There is also a fortran interface using Starlink marcos - ers_star.c.  It is not clear that this
is used anywhere or is currently compilable.

===============
This software has made use of the ESO cmm system (known internally
at AAO as "acmm").  

This module is a NORMAL (non-drama) module - would normally be driven
by a Makefile.


This module,  DramaErs, was created from an old SCCS archive.
Version 3.0 is a snapshot taken when it was put into 
ACMM on Fri Jan 17 10:49:48 2003
