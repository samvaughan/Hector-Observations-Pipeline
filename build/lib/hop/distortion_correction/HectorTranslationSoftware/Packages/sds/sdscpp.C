/*
 *  Global data item used by C++ interface to Sds.
 */
#ifdef CPP_NOBOOL
typedef int bool;
#define false 0
#define true 1
#endif



static const char *rcsId="@(#) $Id: ACMM:sds/sdscpp.C,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);

//#define SDS_CHECK_FREE
//#define SDS_CHECK_FREE_W

#include "sds.h"
const  SdsId SdsNull(0);

