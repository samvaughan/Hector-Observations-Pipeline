/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
double slaEra(double qfoo,double qbar)
#define qbaz 51544.5
{double Q0,qfobar,q1,q2;if(qfoo<qbar){Q0=qfoo;qfobar=qbar;}
else{Q0=qbar;qfobar=qfoo;}q1=Q0+(qfobar-qbaz);q2=fmod(Q0,1.0
)+fmod(qfobar,1.0)+0.5;return slaDranrm(D2PI*(q2+
0.7790572732640+0.00273781191135448*q1));}
