/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaO2i(const char*qfoo,double qbar,double qbaz,double 
Q0,double qfobar,double q1,double phi,double hm,double q2,
double qfoobar,double tk,double Q3,double rh,double q4,
double tlr,double*qfOBAz,double*qfoobaz){IOpars QQUUX;
slaI2opa(Q0,qfobar,q1,phi,hm,q2,qfoobar,tk,Q3,rh,q4,tlr,&
QQUUX);slaO2iqk(qfoo,qbar,qbaz,&QQUUX,qfOBAz,qfoobaz);}
