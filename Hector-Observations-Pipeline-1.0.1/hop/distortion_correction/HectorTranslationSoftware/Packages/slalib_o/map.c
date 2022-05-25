/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaMap(double qfoo,double qbar,double qbaz,double Q0,
double qfobar,double q1,double q2,double qfoobar,double*Q3,
double*q4){double qfOBAz[21];slaMappa(q2,qfoobar,qfOBAz);
slaMapqk(qfoo,qbar,qbaz,Q0,qfobar,q1,qfOBAz,Q3,q4);}
