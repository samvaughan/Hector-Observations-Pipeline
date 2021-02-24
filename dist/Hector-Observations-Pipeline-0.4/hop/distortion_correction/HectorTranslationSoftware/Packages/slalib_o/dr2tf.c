/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaDr2tf(int qfoo,double qbar,char*qbaz,int Q0[4]){
slaDd2tf(qfoo,qbar/D2PI,qbaz,Q0);}
