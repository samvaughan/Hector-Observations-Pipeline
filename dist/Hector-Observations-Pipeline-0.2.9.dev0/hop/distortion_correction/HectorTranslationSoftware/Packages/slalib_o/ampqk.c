/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaAmpqk(double qfoo,double qbar,double qbaz[21],double
*Q0,double*qfobar){double gr2e;double ab1;double ehn[3];
double abv[3];double p[3],q1[3],q2[3],qfoobar[3];double Q3,
q4,qfOBAz,qfoobaz,QQUUX,Q5;int QFRED,qdog;gr2e=qbaz[7];ab1=
qbaz[11];for(QFRED=0;QFRED<3;QFRED++){ehn[QFRED]=qbaz[QFRED+
4];abv[QFRED]=qbaz[QFRED+8];}slaDcs2c(qfoo,qbar,qfoobar);
slaDimxv((double(*)[3])&qbaz[12],qfoobar,q2);Q3=ab1+1.0;for(
QFRED=0;QFRED<3;QFRED++){q1[QFRED]=q2[QFRED];}for(qdog=0;
qdog<2;qdog++){q4=slaDvdv(q1,abv);qfOBAz=1.0+q4;qfoobaz=1.0+
q4/Q3;for(QFRED=0;QFRED<3;QFRED++){q1[QFRED]=(qfOBAz*q2[
QFRED]-qfoobaz*abv[QFRED])/ab1;}slaDvn(q1,qfoobar,&qfoobaz);
for(QFRED=0;QFRED<3;QFRED++){q1[QFRED]=qfoobar[QFRED];}}for(
QFRED=0;QFRED<3;QFRED++){p[QFRED]=q1[QFRED];}for(qdog=0;qdog
<5;qdog++){QQUUX=slaDvdv(p,ehn);Q5=1.0+QQUUX;qfoobaz=Q5-gr2e
*QQUUX;for(QFRED=0;QFRED<3;QFRED++){p[QFRED]=(Q5*q1[QFRED]-
gr2e*ehn[QFRED])/qfoobaz;}slaDvn(p,q2,&qfoobaz);for(QFRED=0;
QFRED<3;QFRED++){p[QFRED]=q2[QFRED];}}slaDcc2s(p,Q0,qfobar);
*Q0=slaDranrm(*Q0);}
