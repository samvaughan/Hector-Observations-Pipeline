/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaRefco(double hm,double tk,double qfoo,double rh,
double qbar,double phi,double tlr,double qbaz,double*refa,
double*refb){double Q0,qfobar;static double q1=
0.7853981633974483;static double q2=1.325817663668033;
slaRefro(q1,hm,tk,qfoo,rh,qbar,phi,tlr,qbaz,&Q0);slaRefro(q2
,hm,tk,qfoo,rh,qbar,phi,tlr,qbaz,&qfobar);*refa=(64.0*Q0-
qfobar)/60.0;*refb=(qfobar-4.0*Q0)/60.0;}
