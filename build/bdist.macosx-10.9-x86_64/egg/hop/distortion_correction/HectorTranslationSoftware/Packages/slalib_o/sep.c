/*
** Copyright (C) 2009 P.T.Wallace.
** Use for profit prohibited - enquiries to ptw@tpsoft.demon.co.uk.
*/
#include "slalib.h"
#include "slamac.h"
float slaSep(float qfoo,float qbar,float qbaz,float Q0){
return(float)slaDsep((double)qfoo,(double)qbar,(double)qbaz,
(double)Q0);}
