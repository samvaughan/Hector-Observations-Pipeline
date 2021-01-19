/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaAoppa(double qfoo,double qbar,double qbaz,double Q0,
double hm,double qfobar,double q1,double tk,double q2,double
 rh,double qfoobar,double tlr,double Q3[14])
#define q4 173.14463331
#define qfOBAz 1.00273790935
{double qfoobaz,QQUUX,Q5,QFRED,qdog,qcat,QFISH,QgASp,phi,Q6,
q7;qfoobaz=cos(Q0);QQUUX=cos(qbaz)*qfoobaz;Q5=sin(qbaz)*
qfoobaz;QFRED=sin(Q0);qdog=QQUUX-qfobar*QFRED;qcat=Q5+q1*
QFRED;QFISH=qfobar*QQUUX-q1*Q5+QFRED;QgASp=(qdog!=0.0||qcat
!=0.0)?atan2(qcat,qdog):0.0;phi=atan2(QFISH,sqrt(qdog*qdog+
qcat*qcat));Q3[0]=phi;Q3[1]=sin(phi);Q3[2]=cos(phi);slaGeoc(
phi,hm,&Q6,&q7);Q3[3]=D2PI*Q6*qfOBAz/q4;Q3[4]=hm;Q3[5]=tk;Q3
[6]=q2;Q3[7]=rh;Q3[8]=qfoobar;Q3[9]=tlr;slaRefco(hm,tk,q2,rh
,qfoobar,phi,tlr,1e-10,&Q3[10],&Q3[11]);Q3[12]=QgASp+
slaEqeqx(qfoo)+qbar*qfOBAz*DS2R;slaAoppat(qfoo,Q3);}
