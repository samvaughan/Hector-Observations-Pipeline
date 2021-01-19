/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaKbj(int qfoo,double qbar,char*qbaz,int*Q0){*Q0=0;if(
qfoo==1){*qbaz='B';}else if(qfoo==2){*qbaz='J';}else if(qfoo
==0){if(qbar<1984.0){*qbaz='B';}else{*qbaz='J';}}else{*qbaz=
' ';*Q0=1;}}
