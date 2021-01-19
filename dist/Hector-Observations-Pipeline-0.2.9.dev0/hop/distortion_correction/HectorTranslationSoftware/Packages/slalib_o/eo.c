/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
double slaEo(double qfoo){double qbar[3][3];slaPneqx(qfoo,
qbar);return slaEors(qbar,slaS(qfoo,qbar[2][0],qbar[2][1]));
}
