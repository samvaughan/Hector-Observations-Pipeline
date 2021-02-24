/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
double slaDsepv(double qfoo[3],double qbar[3]){double qbaz[3
],Q0[3],qfobar,q1;slaDvxv(qfoo,qbar,qbaz);slaDvn(qbaz,Q0,&
qfobar);q1=slaDvdv(qfoo,qbar);return(qfobar!=0.0||q1!=0.0)?
atan2(qfobar,q1):0.0;}
