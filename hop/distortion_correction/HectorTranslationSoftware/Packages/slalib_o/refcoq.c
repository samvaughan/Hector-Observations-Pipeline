/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaRefcoq(double tk,double qfoo,double rh,double wl,
double*refa,double*refb){int qbar;double qbaz,p,Q0,qfobar,q1
,q2,qfoobar,Q3,q4,qfOBAz;qbar=(wl<=100.0);qbaz=gmax(tk,100.0
);qbaz=gmin(qbaz,500.0);p=gmax(qfoo,0.0);p=gmin(p,10000.0);
Q0=gmax(rh,0.0);Q0=gmin(Q0,1.0);qfobar=gmax(wl,0.1);qfobar=
gmin(qfobar,1e6);if(p>0.0){q1=qbaz-273.15;q2=pow(10.0,(
0.7859+0.03477*q1)/(1.0+0.00412*q1))*(1.0+p*(4.5e-6+6e-10*q1
*q1));qfoobar=Q0*q2/(1.0-(1.0-Q0)*q2/p);}else{qfoobar=0.0;}
if(qbar){Q3=qfobar*qfobar;q4=((77.53484e-6+(4.39108e-7+
3.666e-9/Q3)/Q3)*p-11.2684e-6*qfoobar)/qbaz;}else{q4=(
77.6890e-6*p-(6.3938e-6-0.375463/qbaz)*qfoobar)/qbaz;}qfOBAz
=4.4474e-6*qbaz;if(!qbar)qfOBAz-=0.0074*qfoobar*qfOBAz;*refa
=q4*(1.0-qfOBAz);*refb=-q4*(qfOBAz-q4/2.0);}
