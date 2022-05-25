/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaPncio(double qfoo,double qbar[3][3]){double qbaz,Q0,
qfobar,q1,q2,qfoobar,Q3,q4;slaPfw(qfoo,&qbaz,&Q0,&qfobar,&q1
);slaNu(qfoo,&q2,&qfoobar);slaFw2xy(qbaz,Q0,qfobar+q2,q1+
qfoobar,&Q3,&q4);slaG2ixys(Q3,q4,slaS(qfoo,Q3,q4),qbar);}
