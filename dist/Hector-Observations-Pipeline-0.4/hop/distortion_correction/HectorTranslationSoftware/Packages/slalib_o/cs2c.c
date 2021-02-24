/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
void slaCs2c(float qfoo,float qbar,float qbaz[3]){float Q0;
Q0=(float)cos(qbar);qbaz[0]=(float)cos(qfoo)*Q0;qbaz[1]=(
float)sin(qfoo)*Q0;qbaz[2]=(float)sin(qbar);}
