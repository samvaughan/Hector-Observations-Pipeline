/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
#include <limits.h>
void slaInt2in(const char*qfoo,int*qbar,int*qbaz,int*Q0){
long qfobar;qfobar=(long)*qbaz;slaIntin(qfoo,qbar,&qfobar,Q0
);if(*Q0<2){if(qfobar>=(long)INT_MIN&&qfobar<=(long)INT_MAX)
{*qbaz=(int)qfobar;}else{*Q0=2;}}}
