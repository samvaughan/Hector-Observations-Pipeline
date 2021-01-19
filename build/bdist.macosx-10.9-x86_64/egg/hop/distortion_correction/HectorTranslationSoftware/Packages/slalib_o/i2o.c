/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaI2o(double qfoo,double qbar,double qbaz,double Q0,
double qfobar,double phi,double hm,double q1,double q2,
double tk,double qfoobar,double rh,double Q3,double tlr,
double*q4,double*qfOBAz,double*qfoobaz,double*QQUUX,double*
Q5){IOpars QFRED;slaI2opa(qbaz,Q0,qfobar,phi,hm,q1,q2,tk,
qfoobar,rh,Q3,tlr,&QFRED);slaI2oqk(qfoo,qbar,&QFRED,q4,
qfOBAz,qfoobaz,QQUUX,Q5);}
