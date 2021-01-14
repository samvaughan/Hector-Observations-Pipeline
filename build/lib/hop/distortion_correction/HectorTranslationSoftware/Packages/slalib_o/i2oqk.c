/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaI2oqk(double qfoo,double qbar,IOpars*qbaz,double*Q0,
double*qfobar,double*q1,double*q2,double*qfoobar)
#define Q3 0.242535625
{int q4;double qfOBAz[3],qfoobaz,QQUUX,Q5,QFRED,qdog,qcat,
QFISH,QgASp,Q6,q7,q8,QBAD,qBuG,qsilly,QBUGGY,QMUM,qDAd,q9,
Q10,Q11,q12,Q13,Q14,qdisk,Q15;slaDcs2c(qfoo-qbaz->eral,qbar,
qfOBAz);qfoobaz=qfOBAz[0];QQUUX=qfOBAz[1];Q5=qfOBAz[2];QFRED
=qfoobaz+qbaz->xpl*Q5;qdog=QQUUX-qbaz->ypl*Q5;qcat=Q5-qbaz->
xpl*qfoobaz+qbaz->ypl*QQUUX;QFISH=(1.0-qbaz->diurab*qdog);
QgASp=QFISH*QFRED;Q6=QFISH*(qdog+qbaz->diurab);q7=QFISH*qcat
;q8=qbaz->sphi*QgASp-qbaz->cphi*q7;QBAD=Q6;qBuG=qbaz->cphi*
QgASp+qbaz->sphi*q7;qsilly=(q8!=0.0||QBAD!=0.0)?atan2(QBAD,-
q8):0.0;QBUGGY=atan2(sqrt(q8*q8+QBAD*QBAD),qBuG);slaRefz(
QBUGGY,qbaz->refa,qbaz->refb,&QMUM);if(cos(QMUM)<Q3){q4=1;do
{slaRefro(QMUM,qbaz->hm,qbaz->tk,qbaz->p,qbaz->rh,qbaz->wl,
qbaz->phi,qbaz->tlr,1e-8,&q9);qDAd=QMUM+q9-QBUGGY;QMUM-=qDAd
;q4++;}while(fabs(qDAd)>1e-10&&q4<=10);}Q10=sin(QMUM);Q11=-
cos(qsilly)*Q10;q12=sin(qsilly)*Q10;Q13=cos(QMUM);qfOBAz[0]=
qbaz->sphi*Q11+qbaz->cphi*Q13;qfOBAz[1]=q12;qfOBAz[2]=-qbaz
->cphi*Q11+qbaz->sphi*Q13;slaDcc2s(qfOBAz,&Q14,&qdisk);Q15=
slaDranrm(qbaz->eral+Q14);*Q0=qsilly;*qfobar=QMUM;*q1=-Q14;*
q2=qdisk;*qfoobar=Q15;}
