/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaSubet(double qfoo,double qbar,double qbaz,double*Q0,
double*qfobar){double q1[3],q2[3],qfoobar;int Q3;slaEtrms(
qbaz,q1);slaDcs2c(qfoo,qbar,q2);qfoobar=1.0+slaDvdv(q2,q1);
for(Q3=0;Q3<3;Q3++){q2[Q3]=qfoobar*q2[Q3]-q1[Q3];}slaDcc2s(
q2,Q0,qfobar);*Q0=slaDranrm(*Q0);}
