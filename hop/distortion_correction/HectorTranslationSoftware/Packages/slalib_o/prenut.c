/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaPrenut(double qfoo,double qbar,double qbaz[3][3]){
double Q0[3][3],qfobar[3][3];slaPrec(qfoo,slaEpj(qbar),Q0);
slaNut(qbar,qfobar);slaDmxm(qfobar,Q0,qbaz);}
