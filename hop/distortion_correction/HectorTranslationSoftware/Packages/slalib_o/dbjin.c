/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
#include <string.h>
void slaDbjin(const char*qfoo,int*qbar,double*qbaz,int*Q0,
int*qfobar){int q1,q2,qfoobar,Q3,q4,qfOBAz;char qfoobaz;q1=0
;q2=(int)strlen(qfoo);qfoobar=*qbar;slaDfltin(qfoo,&qfoobar,
qbaz,&Q3);if((qfoobar>0)&&(qfoobar<=q2)){if(Q3==1){qfoobaz=
qfoo[qfoobar-1];if(qfoobaz=='B'||qfoobaz=='b'){q1=1;}else if
(qfoobaz=='J'||qfoobaz=='j'){q1=2;}if(q1==1||q1==2){q4=
qfoobar+1;slaDfltin(qfoo,&q4,qbaz,&qfOBAz);if(qfOBAz<=0){
qfoobar=q4;Q3=qfOBAz;}else{q1=0;}}}}*qbar=qfoobar;*Q0=Q3;*
qfobar=q1;}
