#include <string.h>
#include "sds.h"
#include "arg.h"

/*
 * Fortran callable subroutines that invoke C functions
 *
 * These routines map the Fortran routine name to the appropriate SDS C
 * routine. They also handle mapping the Fortran strings to C strings.
 */

/* maximum characters in argument structure item name */
#define MAX_NAME_LEN 24

/* maximum characters in argument structure character string value */
#define MAX_VALUE_LEN 256

/*---------------------------------------------------------------------------*/

void arg_new_(int* p_id,int* p_status)
/*
 *+
 *  Purpose:
 *     Creates a new argument structure.
 *
 *  Description:
 *     ???
 *
 *  Fortran Call:
 *     CALL ARG_NEW(ID,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Returned)
 *        SDS object identifier
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    ArgNew((SdsIdType*)p_id,(StatusType*)p_status);
}

/*---------------------------------------------------------------------------*/
   
void arg_delet_(int* p_id,int* p_status)
/*
 *+
 *  Purpose:
 *     Deletes an argument structure.
 *
 *  Description:
 *     ???
 *
 *  Fortran Call:
 *     CALL ARG_DELET(ID,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Returned)
 *        SDS object identifier
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    ArgDelete((SdsIdType)*p_id,(StatusType*)p_status);
}
   
/*---------------------------------------------------------------------------*/

void arg_get0i_(int* p_id,
		char* name,
		int* p_value,
		int* p_status,
		int len_name)
/*
 *+
 *  Purpose:
 *     Gets an integer item value from an argument structure.
 *
 *  Description:
 *     Returns error if item is not present.
 *
 *  Fortran Call:
 *     CALL ARG_GET0I(ID,NAME,VALUE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VALUE = INTEGER (Returned)
 *        Item value
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    long long_value;

    if (*p_status != STATUS__OK) return;

    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';
  
    ArgGeti((SdsIdType)*p_id,tmp_name,&long_value,(StatusType*)p_status);
     
    *p_value = (int)long_value;
}

/*---------------------------------------------------------------------------*/

void arg_put0i_(int* p_id,
		char* name,
		int* p_value,
                int* p_status,
		int len_name)	       
/*
 *+
 *  Purpose:
 *     Puts an integer value into an argument structure.
 *
 *  Description:
 *     ???
 *
 *  Fortran Call:
 *     CALL ARG_PUT0I(ID,NAME,VALUE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VALUE = INTEGER (Given)
 *        New value for item
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    long long_value;

    if (*p_status != STATUS__OK) return;

    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';

    long_value = (long)*p_value;

    ArgPuti((SdsIdType)*p_id,tmp_name,long_value,(StatusType*)p_status);
}

/*---------------------------------------------------------------------------*/
      
void arg_get0r_(int* p_id,
		char* name,
		float *p_value,
		int* p_status,
		int len_name)
/*
 *+
 *  Purpose:
 *     Gets a real (32-bit floating point) item value from an argument
 *     structure.
 *
 *  Description:
 *     Returns error if item is not present.
 *
 *  Fortran Call:
 *     CALL ARG_GET0R(ID,NAME,VALUE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VALUE = REAL (Returned)
 *        Item value
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    
    if (*p_status != STATUS__OK) return;

    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';

    ArgGetf((SdsIdType)*p_id,tmp_name,p_value,(StatusType*)p_status);
}

/*---------------------------------------------------------------------------*/
  
void arg_put0r_(int* p_id,
		char* name,
		float *p_value,
		int* p_status,
		int len_name)	       
/*
 *+
 *  Purpose:
 *     Puts a real (32-bit floating point) value into an argument structure.
 *
 *  Description:
 *     ???
 *
 *  Fortran Call:
 *     CALL ARG_PUT0R(ID,NAME,VALUE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VALUE = REAL (Given)
 *        New value for item
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    
    if (*p_status != STATUS__OK) return;

    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';

    ArgPutf((SdsIdType)*p_id,tmp_name,*p_value,(StatusType*)p_status);
}

/*---------------------------------------------------------------------------*/
      
void arg_get0d_(int* p_id,
		char* name,
		double *p_value,
		int* p_status,
		int len_name)
/*
 *+
 *  Purpose:
 *     Gets a double precision (64-bit floating point) item value from an
 *     argument structure.
 *
 *  Description:
 *     Returns error if item is not present.
 *
 *  Fortran Call:
 *     CALL ARG_GET0R(ID,NAME,VALUE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VALUE = DOUBLE PRECISION (Returned)
 *        Item value
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    
    if (*p_status != STATUS__OK) return;

    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';

    ArgGetd((SdsIdType)*p_id,tmp_name,p_value,(StatusType*)p_status);
}

/*---------------------------------------------------------------------------*/
  
void arg_put0d_(int* p_id,
		char* name,
		double *p_value,
		int* p_status,
		int len_name)	       
/*
 *+
 *  Purpose:
 *     Puts a double precision (64-bit floating point) value into an argument
 *     structure.
 *
 *  Description:
 *     ???
 *
 *  Fortran Call:
 *     CALL ARG_PUT0R(ID,NAME,VALUE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VALUE = DOUBLE PRECISION (Given)
 *        New value for item
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    
    if (*p_status != STATUS__OK) return;

    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';

    ArgPutd((SdsIdType)*p_id,tmp_name,*p_value,(StatusType*)p_status);
}

/*---------------------------------------------------------------------------*/
      
void arg_get0c_(int* p_id,
		char* name,
		char* value,
		int* p_status,
		int len_name,
		int len_value)
	       
/*
 *+
 *  Purpose:
 *     Gets a character string item value from an argument structure.
 *
 *  Description:
 *     Returns error if item is not present.
 *
 *  Fortran Call:
 *     CALL ARG_GET0C(ID,NAME,VALUE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VALUE = CHARACTER*(*) (Returned)
 *        Item value
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    char tmp_value[MAX_VALUE_LEN];
    int i;

    if (*p_status != STATUS__OK) return;

    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';

    ArgGetString((SdsIdType)*p_id,tmp_name,
		 MAX_VALUE_LEN,tmp_value,(StatusType*)p_status);

    if (*p_status != STATUS__OK) return;

    strncpy(value,tmp_value,len_value);
    for (i=strlen(value);i<len_value;i++)
	{
	value[i] = ' ';
	}
}

/*---------------------------------------------------------------------------*/

void arg_put0c_(int* p_id,
		char* name, 
                char* value,
                int* p_status,
		int len_name,
		int len_value)
/*
 *+
 *  Purpose:
 *     Puts a character string value into an argument structure.
 *
 *  Description:
 *     ???
 *
 *  Fortran Call:
 *     CALL ARG_PUT0C(ID,NAME,VALUE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VALUE = CHARACTER*(*) (Given)
 *        New value for item
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    char tmp_value[MAX_VALUE_LEN];

    if (*p_status != STATUS__OK) return;

    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';

    strncpy(tmp_value,value,len_value);
    tmp_value[len_value] = '\0';

    ArgPutString((SdsIdType)*p_id,tmp_name,tmp_value,(StatusType*)p_status);
}

/*---------------------------------------------------------------------------*/

void arg_putvd_(int*    p_id,
		 char*   name,
		 double* p_vec,
                 int*    p_size,
                 int*    p_status,
		 int     len_name)	       
/*
 *+
 *  Purpose:
 *     Puts a vector of doubles into specified argument structure.
 *
 *  Description:
 *     Creates a 1 dimensional array structure of v_size doubles
 *     via the  the SdsNew command and then populates the new strcuture
 *     via SdsPut
 *
 *  Fortran Call:
 *     CALL ARG_PUT0DV(ID,NAME,VECTOR,SIZE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VECTOR(SIZE) = DOUBLE (Given)
 *        New value for item
 *     SIZE = INTEGER (Given)
 *        Size of double precision vector
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    long long_value; 
    unsigned long dims[1];
    SdsIdType id2;

    if (*p_status != STATUS__OK) return;

    /*Copy the specified name from fortran definition into usable c string*/
    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';

    /*Allocate a new array structure of doubles*/
    dims[0]=*p_size;
    SdsNew((SdsIdType)*p_id,tmp_name,0,NULL,SDS_DOUBLE,1,dims,&id2,(StatusType*)p_status);
   
    if (*p_status != STATUS__OK) return;

    /*Populate the array structure with values from the supplied vector*/
    SdsPut(id2,dims[0]*sizeof(double),0,p_vec,(StatusType*)p_status);


}

/*---------------------------------------------------------------------------*/

void arg_putvr_(int*    p_id,
		 char*   name,
		 float* p_vec,
                 int*    p_size,
                 int*    p_status,
		 int     len_name)	       
/*
 *+
 *  Purpose:
 *     Puts a vector of floats (fortran reals) into specified argument
 *     structure.
 *
 *  Description:
 *     Creates a 1 dimensional array structure of v_size doubles
 *     via the  the SdsNew command and then populates the new strcuture
 *     via SdsPut
 *
 *  Fortran Call:
 *     CALL ARG_PUT0RV(ID,NAME,VECTOR,SIZE,STATUS)
 *
 *  Arguments:
 *     ID = INTEGER (Given)
 *        SDS object identifier
 *     NAME = CHARACTER*(*) (Given)
 *        Item name
 *     VECTOR(SIZE) = REAL (Given)
 *        New value for item
 *     SIZE = INTEGER (Given)
 *        Size of real vector
 *     STATUS = INTEGER (Given and Returned)
 *        Global status
 *-
 */
{
    char tmp_name[MAX_NAME_LEN];
    long long_value; 
    unsigned long dims[1];
    SdsIdType id2;

    if (*p_status != STATUS__OK) return;

    /*Copy the specified name from fortran definition into usable c string*/
    strncpy(tmp_name,name,len_name);
    tmp_name[len_name] = '\0';

    /*Allocate a new array structure of doubles*/
    dims[0]=*p_size;
    SdsNew((SdsIdType)*p_id,tmp_name,0,NULL,SDS_FLOAT,1,dims,&id2,(StatusType*)p_status);
   
    if (*p_status != STATUS__OK) return;

    /*Populate the array structure with values from the supplied vector*/
    SdsPut(id2,dims[0]*sizeof(float),0,p_vec,(StatusType*)p_status);


}
