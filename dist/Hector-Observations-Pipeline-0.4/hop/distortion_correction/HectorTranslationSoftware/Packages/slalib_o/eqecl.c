/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaEqecl(double qfoo,double qbar,double qbaz,double*Q0,
double*qfobar){double q1[3][3],q2[3],qfoobar[3];slaDcs2c(
qfoo,qbar,q2);slaPrec(2000.0,slaEpj(qbaz),q1);slaDmxv(q1,q2,
qfoobar);slaEcmat(qbaz,q1);slaDmxv(q1,qfoobar,q2);slaDcc2s(
q2,Q0,qfobar);*Q0=slaDranrm(*Q0);*qfobar=slaDrange(*qfobar);
}
