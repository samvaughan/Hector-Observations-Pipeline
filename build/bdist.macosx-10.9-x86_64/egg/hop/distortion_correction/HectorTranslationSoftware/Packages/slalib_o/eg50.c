/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaEg50(double qfoo,double qbar,double*qbaz,double*Q0){
double qfobar[3],q1[3],q2,qfoobar;static double Q3[3][3]={{-
0.066988739415151,-0.872755765851993,-0.483538914632184},{
0.492728466075324,-0.450346958019961,0.744584633283031},{-
0.867600811151435,-0.188374601722920,0.460199784783852}};
slaSubet(qfoo,qbar,1950.0,&q2,&qfoobar);slaDcs2c(q2,qfoobar,
qfobar);slaDmxv(Q3,qfobar,q1);slaDcc2s(q1,qbaz,Q0);*qbaz=
slaDranrm(*qbaz);*Q0=slaDrange(*Q0);}
