/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaDtf2r(int qfoo,int qbar,double qbaz,double*Q0,int*
qfobar){double q1;slaDtf2d(qfoo,qbar,qbaz,&q1,qfobar);*Q0=
D2PI*q1;}
