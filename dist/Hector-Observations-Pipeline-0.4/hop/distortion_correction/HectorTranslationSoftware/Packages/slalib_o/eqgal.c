/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaEqgal(double qfoo,double qbar,double*qbaz,double*Q0)
{double qfobar[3],q1[3];static double q2[3][3]={{-
0.054875539695716,-0.873437107995315,-0.483834985836994},{
0.494109453305607,-0.444829589431879,0.746982251810510},{-
0.867666135847849,-0.198076386130820,0.455983795721093}};
slaDcs2c(qfoo,qbar,qfobar);slaDmxv(q2,qfobar,q1);slaDcc2s(q1
,qbaz,Q0);*qbaz=slaDranrm(*qbaz);*Q0=slaDrange(*Q0);}
