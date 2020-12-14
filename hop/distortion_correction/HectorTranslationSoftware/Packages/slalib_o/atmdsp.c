/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaAtmdsp(double tk,double qfoo,double rh,double qbar,
double qbaz,double Q0,double qfobar,double*q1,double*q2){
double qfoobar,Q3,q4,qfOBAz,qfoobaz,QQUUX,Q5,QFRED,qdog,qcat
,QFISH,QgASp;if(qbar<0.0||qfobar<0.0||qbar>100.0||qfobar>
100.0){*q1=qbaz;*q2=Q0;}else{qfoobar=gmax(tk,100.0);qfoobar=
gmin(qfoobar,500.0);Q3=gmax(qfoo,0.0);Q3=gmin(Q3,10000.0);q4
=gmax(rh,0.0);q4=gmin(q4,1.0);qfOBAz=pow(10.0,-8.7115+
0.03477*qfoobar);qfoobaz=q4*qfOBAz;QQUUX=11.2684e-6*qfoobaz;
Q5=gmax(qbar,0.1);QFRED=Q5*Q5;qdog=77.5317e-6+(0.43909e-6+
0.00367e-6/QFRED)/QFRED;qcat=(qdog*Q3-QQUUX)/qfoobar;Q5=gmax
(qfobar,0.1);QFRED=Q5*Q5;qdog=77.5317e-6+(0.43909e-6+
0.00367e-6/QFRED)/QFRED;QFISH=(qdog*Q3-QQUUX)/qfoobar;if(
qcat!=0.0){QgASp=QFISH/qcat;*q1=qbaz*QgASp;*q2=Q0*QgASp;if(
qcat!=qbaz)*q2=*q2*(1.0+qcat*(qcat-QFISH)/(2.0*(qcat-qbaz)))
;}else{*q1=qbaz;*q2=Q0;}}}
