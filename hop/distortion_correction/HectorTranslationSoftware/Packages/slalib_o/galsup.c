/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaGalsup(double qfoo,double qbar,double*qbaz,double*Q0
){double qfobar[3],q1[3];static double q2[3][3]={{-
0.735742574804,0.677261296414,0.0},{-0.074553778365,-
0.080991471307,0.993922590400},{0.673145302109,
0.731271165817,0.110081262225}};slaDcs2c(qfoo,qbar,qfobar);
slaDmxv(q2,qfobar,q1);slaDcc2s(q1,qbaz,Q0);*qbaz=slaDranrm(*
qbaz);*Q0=slaDrange(*Q0);}
