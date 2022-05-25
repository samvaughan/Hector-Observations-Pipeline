/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaAmp(double qfoo,double qbar,double qbaz,double Q0,
double*qfobar,double*q1){double q2[21];slaMappa(Q0,qbaz,q2);
slaAmpqk(qfoo,qbar,q2,qfobar,q1);}
