/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaI2c(double qfoo,double qbar,double qbaz,double Q0,
double qfobar,double*q1,double*q2){CIpars qfoobar;double Q3;
slaC2ipa(qbaz,Q0,qfobar,&qfoobar,&Q3);slaI2cqk(qfoo,qbar,&
qfoobar,q1,q2);}
