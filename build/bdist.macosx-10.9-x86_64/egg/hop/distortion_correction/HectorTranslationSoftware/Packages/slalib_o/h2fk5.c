/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaH2fk5(double rh,double qfoo,double qbar,double qbaz,
double*Q0,double*qfobar,double*q1,double*q2)
#define qfoobar 0.484813681109535994e-5
{static double Q3[3]={-19.9e-3*qfoobar,-9.1e-3*qfoobar,
22.9e-3*qfoobar},q4[3]={-0.30e-3*qfoobar,0.60e-3*qfoobar,
0.70e-3*qfoobar};double qfOBAz[6],qfoobaz[3][3],QQUUX[3],Q5[
3],QFRED[6],qdog,qcat,QFISH;int QgASp;slaDs2c6(rh,qfoo,1.0,
qbar,qbaz,0.0,qfOBAz);slaDav2m(Q3,qfoobaz);slaDmxv(qfoobaz,
q4,QQUUX);slaDimxv(qfoobaz,qfOBAz,QFRED);slaDvxv(qfOBAz,
QQUUX,Q5);for(QgASp=0;QgASp<3;QgASp++){Q5[QgASp]=qfOBAz[
QgASp+3]-Q5[QgASp];}slaDimxv(qfoobaz,Q5,QFRED+3);slaDc62s(
QFRED,&qdog,qfobar,&qcat,q1,q2,&QFISH);*Q0=slaDranrm(qdog);}
