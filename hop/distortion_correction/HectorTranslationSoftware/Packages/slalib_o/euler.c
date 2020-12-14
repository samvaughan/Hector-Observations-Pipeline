/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaEuler(const char*qfoo,float phi,float qbar,float 
qbaz,float Q0[3][3]){int qfobar,q1;double q2[3][3];slaDeuler
(qfoo,(double)phi,(double)qbar,(double)qbaz,q2);for(qfobar=0
;qfobar<3;qfobar++){for(q1=0;q1<3;q1++){Q0[q1][qfobar]=(
float)q2[q1][qfobar];}}}
