/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaAddet(double qfoo,double qbar,double qbaz,double*Q0,
double*qfobar){double q1[3];double q2[3];int qfoobar;
slaEtrms(qbaz,q1);slaDcs2c(qfoo,qbar,q2);for(qfoobar=0;
qfoobar<3;qfoobar++){q2[qfoobar]+=q1[qfoobar];}slaDcc2s(q2,
Q0,qfobar);*Q0=slaDranrm(*Q0);}
