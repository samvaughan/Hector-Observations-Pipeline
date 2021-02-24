/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaCtf2r(int qfoo,int qbar,float qbaz,float*Q0,int*
qfobar){double q1;slaDtf2d(qfoo,qbar,(double)qbaz,&q1,qfobar
);*Q0=(float)(q1*D2PI);}
