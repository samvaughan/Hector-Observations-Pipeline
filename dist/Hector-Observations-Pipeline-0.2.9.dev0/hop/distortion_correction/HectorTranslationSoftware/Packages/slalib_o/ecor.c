/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaEcor(float qfoo,float qbar,int qbaz,int Q0,float 
qfobar,float*q1,float*q2)
#define qfoobar 1.4959787066e8f
#define Q3 499.0047837f
{float q4[6],qfOBAz[3];slaEarth(qbaz,Q0,qfobar,q4);slaCs2c(
qfoo,qbar,qfOBAz);*q1=-qfoobar*slaVdv(&q4[3],qfOBAz);*q2=Q3*
slaVdv(q4,qfOBAz);}
