/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaI2opa(double qfoo,double qbar,double qbaz,double phi
,double hm,double Q0,double qfobar,double tk,double q1,
double rh,double q2,double tlr,IOpars*qfoobar)
#define Q3 173.14463331
#define q4 1.00273781191135448
{double qfOBAz,qfoobaz,QQUUX,Q5;qfoobar->along=qbaz+slaSp(
qfoo)+qbar*q4*DS2R;qfoobar->phi=phi;qfoobar->hm=hm;qfOBAz=
sin(qbaz);qfoobaz=cos(qbaz);qfoobar->xpl=Q0*qfoobaz-qfobar*
qfOBAz;qfoobar->ypl=Q0*qfOBAz+qfobar*qfoobaz;qfoobar->sphi=
sin(phi);qfoobar->cphi=cos(phi);slaGeoc(phi,hm,&QQUUX,&Q5);
qfoobar->diurab=D2PI*QQUUX*q4/Q3;qfoobar->p=q1;qfoobar->tk=
tk;qfoobar->rh=rh;qfoobar->tlr=tlr;qfoobar->wl=q2;slaRefco(
qfoobar->hm,qfoobar->tk,qfoobar->p,qfoobar->rh,qfoobar->wl,
qfoobar->phi,qfoobar->tlr,1e-10,&qfoobar->refa,&qfoobar->
refb);slaI2opat(qfoo,qfoobar);}
