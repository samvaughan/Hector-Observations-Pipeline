/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaC2iqk(double qfoo,double qbar,double qbaz,double Q0,
double qfobar,double q1,CIpars*q2,double*qfoobar,double*Q3)
#define q4 0.21094502
{int qfOBAz;double qfoobaz[3],QQUUX,Q5,QFRED[3],p[3],qdog[3]
,qcat,QFISH,QgASp[3],Q6,q7[3],q8[3];slaDcs2c(qfoo,qbar,
qfoobaz);QQUUX=qfobar*DAS2R;Q5=q4*q1*QQUUX;QFRED[0]=-qbaz*
qfoobaz[1]-Q0*cos(qfoo)*sin(qbar)+Q5*qfoobaz[0];QFRED[1]=
qbaz*qfoobaz[0]-Q0*sin(qfoo)*sin(qbar)+Q5*qfoobaz[1];QFRED[2
]=Q0*cos(qbar)+Q5*qfoobaz[2];for(qfOBAz=0;qfOBAz<3;qfOBAz++)
{p[qfOBAz]=qfoobaz[qfOBAz]+q2->pmt*QFRED[qfOBAz]-QQUUX*q2->
eb[qfOBAz];}slaDvn(p,qdog,&Q5);qcat=slaDvdv(qdog,q2->ehn);
QFISH=qcat+1.0;Q5=q2->gr2e/gmax(QFISH,1.0e-5);for(qfOBAz=0;
qfOBAz<3;qfOBAz++){QgASp[qfOBAz]=qdog[qfOBAz]+Q5*(q2->ehn[
qfOBAz]-qcat*qdog[qfOBAz]);}Q6=slaDvdv(QgASp,q2->abv);Q5=1.0
+Q6/(q2->ab1+1.0);for(qfOBAz=0;qfOBAz<3;qfOBAz++){q7[qfOBAz]
=q2->ab1*QgASp[qfOBAz]+Q5*q2->abv[qfOBAz];}slaDmxv(q2->bpn,
q7,q8);slaDcc2s(q8,qfoobar,Q3);*qfoobar=slaDranrm(*qfoobar);
}
