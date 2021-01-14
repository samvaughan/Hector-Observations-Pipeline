/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaCalyd(int qfoo,int qbar,int qbaz,int*Q0,int*qfobar,
int*q1){int q2;if((qfoo>=0)&&(qfoo<=49))q2=qfoo+2000;else if
((qfoo>=50)&&(qfoo<=99))q2=qfoo+1900;else q2=qfoo;slaClyd(q2
,qbar,qbaz,Q0,qfobar,q1);}
