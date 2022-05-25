

static const char *rcsId="@(#) $Id: ACMM:sds/sdsutil.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "status.h"
#define _SDS_UTIL_BUILD  /* Used to enable extra calls in sds.h */
#include "sds.h"
#include "arg_err.h"
/* 
 *     @(#) $Id: ACMM:sds/sdsutil.c,v 3.94 09-Dec-2020 17:15:24+11 ks $
 */


/*+				S d s W r i t e

 *  Function name:
      SdsWrite

 *  Function:
      Write an SDS object to a file

 *  Description:
      Given an identifier to an internal SDS object, write it to
      a file. The file can be read back using SdsRead.
      
 *  Language:
      C

 *  Declaration:
      void SdsWrite(SdsIdType id, char *filename, StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  The identifier to the object to be output
      (>) filename  (char*) The name of the file into which the object
                              will be written
      (!) status    (long*) Modified status. Possible failure codes are:
                        SDS__BADID => The identifier is invalid
                        SDS__EXTERN => The object is external
                        SDS__NOMEM  => Insufficient memory for output buffer
                        SDS__FOPEN  => Error opening the file
                        SDS__FWRITE => Error writing the file


 *  Prior requirements:
      None.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 31-Jan-92
 *-
 */


void SdsWrite(SdsIdType id, SDSCONST char* filename, StatusType * SDSCONST status)

{
   unsigned long size;
   void *ptr;
   FILE *output;
   int nw;

/*  Get size of buffer required to export the object  */

   SdsSize(id,&size,status);
   if (*status != SDS__OK) return;

/*  Allocate memory as buffer  */

   ptr = (void*) SdsMalloc(size);

/*  Check for errors  */

   if (ptr == 0)
     {
        *status = SDS__NOMEM;
        return;
     } 

/*  Export the SDS object into the buffer  */

   SdsExport(id,size,ptr,status);

/*  Return if there were errors  */

   if (*status != SDS__OK) return;

/*  Open the output file  */

   output = fopen(filename,"wb");
   if (output == NULL)
      *status = SDS__FOPEN;
   else

/*  If no errors write the data to the output file   */

     {
       nw = fwrite(ptr,size,1,output);
       if (nw == 0)
          *status = SDS__FWRITE;
       if (fclose(output) != 0)
          *status = SDS__FWRITE;
     } 
   SdsFree(ptr); 
}


UINT32 vswap4(unsigned long i)

{
   UINT32 temp;

     {
       temp = (i<<24) | ((i<<8) & 0xFF0000) | ((i>>8) & 0xFF00) | (i>>24);
       return(temp);
     }
}





/*+				S d s R e a d

 *  Function name:
      SdsRead

 *  Function:
      Read an SDS object from a file

 *  Description:
      Read an SDS object from a file previously written by SdsWrite.
      An identifier to an external object is returned. If an internal
      version of the object is required it can be created using SdsCopy.
      
 *  Language:
      C

 *  Declaration:
      void SdsRead(char *filename, SdsIdType *id, StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) filename  (char*) The name of the file from which the object
                              will be read.
      (<) id        (SdsIdType*) The identifier to the external object.
      (!) status    (long*) Modified status. Possible failure codes are:

                        SDS__NOTSDS => Not a valid SDS object
                        SDS__NOMEM  => Insufficient memory
                        SDS__FOPEN  => Error opening the file
                        SDS__FREAD  => Error reading the file

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 4-Feb-92
 *-
 */


void SdsRead(SDSCONST char *filename, SdsIdType *id, StatusType * SDSCONST status)

{
  FILE *input;
  unsigned long size;
  unsigned long version;
  void *ptr;
  UINT32 buf[3];
  int nr;

/*  Return if status bad on entry  */

  if (*status != SDS__OK) return;

/*  Open the file and check for errors  */

  input = fopen(filename,"rb");
  if (input == NULL)
    {
      *status = SDS__FOPEN;
      return;
    }

/*  Read the first three longwords of the file   */

  nr = fread(buf,4,3,input);
  if (nr == 0)
    {
      *status = SDS__FREAD;
      return;
    }

/* 
 * First longword indicates the byte order
 * If necessary byte swap the second longword which is the file length  and
 * the third which is the SDS version.
 */

  if ((buf[0] == 0) != SDS_BIGEND)
  {
     size    = vswap4(buf[1]);
     version = vswap4(buf[2]);
  }
  else
  {
     size    = buf[1];
     version = buf[2];
  }

/*  
 *  Close the file 
 */
  fclose(input);
/*
 * Check the SDS version.
 */
  if (version != SDS_VERSION)
  {
      *status = SDS__NOTSDS;
      return;
  }
/*
 * Reopen the file.
 */
  input = fopen(filename,"rb");
  if (input == NULL)
    {
      *status = SDS__FOPEN;
      return;
    }

/*  Get a buffer large enough to hold the object  */

  ptr = (void*) SdsMalloc(size);
  if (ptr == 0)
    {
      *status = SDS__NOMEM;
      return;
    }

/*  Read into the buffer from the file  */

  nr = fread(ptr,size,1,input);
  fclose(input);
  if (nr == 0)
  {
      SdsFree(ptr);
      *status = SDS__FREAD;
      return;
  }

/*  Get an SDS identifier to the object  */

  SdsAccess(ptr,id,status);
  if (*status == STATUS__OK)
      Sds__MarkFree(*id);
  else
      SdsFree(ptr);
}


/*+				S d s R e a d F r e e

 *  Function name:
      SdsReadFree

 *  Function:
      Free Buffer allocated by SdsRead

 *  Description:
      SdsRead allocates a block of memory to hold the external object read in.
      This memory can be released when the object is no longer required by calling
      SdsReadFree (note that it is not possible to SdsDelete an external object).

      If SdsReadFree is given an idientifier which was not produced by a call to
      SdsRead it will do nothing.
      
 *  Language:
      C

 *  Declaration:
      void SdsReadFree(SdsIdType id, StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (<) id        (SdsIdType*) The identifier to the external object.
      (!) status    (long*) Modified status. Possible failure codes are:

                        SDS__NOTSDS => Not a valid SDS object
                        SDS__NOMEM  => Insufficient memory


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 28-Aug-95
 *-
 */


void SdsReadFree(SdsIdType id, StatusType * SDSCONST status)

{
   Sds___WatchEvent(id, SDS_WATCH_EVENT_READFREE, "SdsReadFree");

   Sds__Free(id,status);
}


static void Sds__list(FILE *to, long level, long id, char *pstr, StatusType * SDSCONST status)

{
  long code;
  long ndims;
  unsigned long dims[SDS_C_MAXARRAYDIMS];
  long size[SDS_C_MAXARRAYDIMS];
  unsigned long index[SDS_C_MAXARRAYDIMS];
  long temp;
  char name[SDS_C_NAMELEN];
  char tname[22];
  char subs[60];
  long ind;
  long i;
  long nid;
  char strtemp[10];
  char cdata;
  char cbuf[200];
  char bdata;
  unsigned char ubdata;
  short sdata;
  unsigned short usdata;
  INT32 ldata;
  UINT32 uldata;
  INT64 l64data;
  UINT64 u64data;
#ifndef NATIVE__INT64
 /*
  *These are only needed  if we don't have native 64 bit integers.
  */
  long high;
  unsigned long uhigh,low;
#endif
  float fdata;
  double ddata;
  long lpos;
  unsigned long actlen;
  char ell[4];
  char buffer[81];
  long ncomps;
  long offset;
  char extraData[70];
  unsigned long extraLen;

/*  Get information on object  */

  lpos = 0;
  SdsInfo(id,name,&code,&ndims,dims,status);
  SdsGetExtra(id, sizeof(extraData)-1, extraData, &extraLen, status);
  if (*status != 0)
    {
       return;
    }
  extraData[extraLen] = 0;

/*  Output the name  */

  for(i=0;i<level;i++) 
    {  
        fprintf(to, "  ");
        lpos = lpos+2;
    }
  strcpy(tname,name);
  strncat(tname,pstr,21-strlen(tname));
  for(i=0;i<21;i++) 
    if (tname[i] == 0)
      {
         tname[i] = ' ';
         tname[i+1] = 0;
      }
  fprintf(to, "%s", tname);
  lpos = lpos+strlen(tname);

/*  Output the type  */

  if (code == SDS_STRUCT)      fprintf(to, "Struct");
  else if (code == SDS_CHAR)   fprintf(to, "Char  ");
  else if (code == SDS_BYTE)   fprintf(to, "Byte  ");
  else if (code == SDS_UBYTE)  fprintf(to, "Ubyte ");
  else if (code == SDS_SHORT)  fprintf(to, "Short ");
  else if (code == SDS_USHORT) fprintf(to, "Ushort");
  else if (code == SDS_INT)    fprintf(to, "Int   ");
  else if (code == SDS_UINT)   fprintf(to, "Uint  ");
  else if (code == SDS_I64)    fprintf(to, "Int64 ");
  else if (code == SDS_UI64)   fprintf(to, "Uint64");
  else if (code == SDS_FLOAT)  fprintf(to, "Float ");
  else if (code == SDS_DOUBLE) fprintf(to, "Double");
  lpos = lpos+6;

/*  Output the dimensions  */

  if (ndims != 0)
    {
       sprintf(buffer," [%ld",dims[0]);
       fprintf(to, "%s", buffer);
       lpos = lpos+strlen(buffer);
       for (i=1;i<ndims;i++) 
         {
            sprintf(buffer,",%ld",dims[i]);
            fprintf(to, "%s", buffer);
            lpos = lpos+strlen(buffer);
         }
       fprintf(to, "]");
       lpos = lpos+1;
    }

/*  If it is a structure loop through the components 
    calling Sds__list for each one  */

  if ((code == SDS_STRUCT) & (ndims == 0))
    {
       fprintf(to, "\n");
       if (extraLen > 0)
           fprintf(to, "[%s]\n", extraData);
       ind = 1;
       while (*status == 0)
         {
            SdsIndex(id,ind,&nid,status);
            if (*status == 0) Sds__list(to, level+1,nid,"",status);
            SdsFreeId(nid,status);
            ind = ind+1;
         }
       *status = 0;
    }

/*  If it is a structure array loop through the array components 
    calling Sds__list for each one  */

  else if ((code == SDS_STRUCT) & (ndims != 0))
    {
       fprintf(to, "\n");
       if (extraLen > 0)
           fprintf(to, "[%s]\n", extraData);
       ind = 0;
       while (*status == 0)
         {
            size[0]=1;
            for(i=1;i<ndims;i++) size[i] = dims[i-1]*size[i-1];
            temp = ind;
            for(i=ndims-1;i>=0;i--)
              {
                 index[i] = temp / size[i] + 1;
                 temp = temp % size[i];
              }
            sprintf(subs,"[%ld",index[0]);
            for (i=1;i<ndims;i++)
              { 
                 sprintf(strtemp,",%ld",index[i]);
                 strcat(subs,strtemp);
              }
            strcat(subs,"]");
            SdsCell(id,ndims,index,&nid,status);
            if (*status == 0) Sds__list(to, level+1,nid,subs,status);
            SdsFreeId(nid,status);
            ind = ind+1;
         }
       *status = 0;
    }
  else

    {

/*  If it is a primitive output the data  */

      SdsGet(id,1,0,(void*)&cdata,&actlen,status);
      if (*status == SDS__UNDEFINED)
        {
           fprintf(to, " Undefined\n");
           if (extraLen > 0)
               fprintf(to, "[%s]\n", extraData);
           *status = 0;
           return;
        }
      else if (*status != 0)
        {
           fprintf(to, "\n Error - status = %ld\n",(long int)*status);
           return;
        }
      if (ndims == 0) 
        {
           switch (code)
             {
               case SDS_CHAR:
                 SdsGet(id,1,0,(void*)&cdata,&actlen,status);
                 fprintf(to, " \"%c\"\n",cdata);
                 break;
               case SDS_UBYTE:
                 SdsGet(id,1,0,(void*)&ubdata,&actlen,status);
                 fprintf(to, " %u\n",ubdata);
                 break;
               case SDS_BYTE:
                 SdsGet(id,1,0,(void*)&bdata,&actlen,status);
                 fprintf(to, " %d\n",bdata);
                 break;
               case SDS_USHORT:
                 SdsGet(id,2,0,(void*)&usdata,&actlen,status);
                 fprintf(to, " %u\n",usdata);
                 break;
               case SDS_SHORT:
                 SdsGet(id,2,0,(void*)&sdata,&actlen,status);
                 fprintf(to, " %d\n",sdata);
                 break;
               case SDS_UINT:
                 SdsGet(id,4,0,(void*)&uldata,&actlen,status);
                 fprintf(to, " %lu\n",(unsigned long)uldata);
                 break;
               case SDS_INT:
                 SdsGet(id,4,0,(void*)&ldata,&actlen,status);
                 fprintf(to, " %ld\n",(long)ldata);
                 break;
               case SDS_UI64:
                 SdsGet(id,8,0,(void*)&u64data,&actlen,status);
#ifdef LONG__64
                 fprintf(to, " %lu\n",u64data);
#elif defined( NATIVE__INT64 )
                 fprintf(to, " %llu\n",u64data);
#else
                 SdsGetUI64(u64data,&uhigh,&low);
                 ddata = ((double)uhigh)*SDS__ULONG_DIVSOR_D + (double)low;
                 fprintf(to, " %.15g\n",ddata);
#endif
                 break;
               case SDS_I64:
                 SdsGet(id,8,0,(void*)&l64data,&actlen,status);
#ifdef LONG__64
                 fprintf(to, " %ld\n",l64data);
#elif defined( NATIVE__INT64 )
                 fprintf(to, " %lld\n",l64data);
#else
                 SdsGetI64(l64data,&high,&low);
                 ddata = ((double)high)*SDS__ULONG_DIVSOR_D + (double)low;
                 fprintf(to, " %.15g\n",ddata);
#endif
                 break;
               case SDS_FLOAT:
                 SdsGet(id,4,0,(void*)&fdata,&actlen,status);
                 fprintf(to, " %g\n",fdata);
                 break;
               case SDS_DOUBLE:
                 SdsGet(id,8,0,(void*)&ddata,&actlen,status);
                 fprintf(to, " %.10g\n",ddata);
                 break;
               default:
                 fprintf(to, "\n Unexpected Type Code - %ld \n",code);
                 *status = 1;
                 return;
            }
           if (extraLen > 0)
               fprintf(to, "[%s]\n", extraData);
        }
      else
        {
           ncomps = 1;
           for(i=0;i<ndims;i++)
             ncomps = ncomps*dims[i];
           if (code == SDS_CHAR)
             {
               if (lpos>70) 
                 {
                    printf("\n");
                    lpos = 0;
                 }
               if (ncomps>(73-lpos)) 
                 {
                   ncomps = 73-lpos;
                   strcpy(ell,"...");
                 }
               else
                 ell[0]=0;
               SdsGet(id,ncomps,0,(void*)cbuf,&actlen,status);
               strncpy(buffer,cbuf,ncomps);
               buffer[ncomps] = 0;
               fprintf(to, " \"%s",buffer);
               fprintf(to, "%s\"\n",ell);
               if (extraLen > 0)
                   fprintf(to, "[%s]\n", extraData);
             }
           else
             {
               fprintf(to, " {");
               lpos = lpos+2;
               for(offset=0;offset<ncomps;offset++)
                 {
                   switch (code)
                     {
                     case SDS_UBYTE:                 
                       SdsGet(id,1,offset,(void*)&ubdata,&actlen,status);
                       sprintf(buffer,"%u",ubdata);
                       break;
                     case SDS_BYTE:                 
                       SdsGet(id,1,offset,(void*)&bdata,&actlen,status);
                       sprintf(buffer,"%d",bdata);
                       break;
                     case SDS_USHORT:                 
                       SdsGet(id,2,offset,(void*)&usdata,&actlen,status);
                       sprintf(buffer,"%u",usdata);
                       break;
                     case SDS_SHORT:                 
                       SdsGet(id,2,offset,(void*)&sdata,&actlen,status);
                       sprintf(buffer,"%d",sdata);
                       break;
                     case SDS_UINT:                 
                       SdsGet(id,4,offset,(void*)&uldata,&actlen,status);
                       sprintf(buffer,"%lu",(unsigned long)uldata);
                       break;
                     case SDS_INT:                 
                       SdsGet(id,4,offset,(void*)&ldata,&actlen,status);
                       sprintf(buffer,"%ld",(long)ldata);
                       break;
                     case SDS_UI64:  
                       SdsGet(id,8,offset,(void*)&u64data,&actlen,status);
#ifdef LONG__64               
                       sprintf(buffer,"%lu",u64data);
#elif defined( NATIVE__INT64 )
		       sprintf(buffer," %llu\n",u64data);
#else
                       SdsGetUI64(u64data,&uhigh,&low);
                       ddata = ((double)uhigh)*SDS__ULONG_DIVSOR_D + (double)low;
                       fprintf(to, " %.15g\n",ddata);
#endif
                       break;
                     case SDS_I64:     
                       SdsGet(id,8,offset,(void*)&l64data,&actlen,status);
#ifdef LONG__64            
                       sprintf(buffer,"%ld",l64data);
#elif defined( NATIVE__INT64 )
		       sprintf(buffer," %lld\n",l64data);
#else
                       SdsGetI64(l64data,&high,&low);
                       ddata = ((double)high)*SDS__ULONG_DIVSOR_D + (double)low;
                       fprintf(to, " %.15g\n",ddata);
#endif
                       break;
                     case SDS_FLOAT:                 
                       SdsGet(id,4,offset,(void*)&fdata,&actlen,status);
                       sprintf(buffer," %g",fdata);
                       break;
                     case SDS_DOUBLE:                 
                       SdsGet(id,8,offset,(void*)&ddata,&actlen,status);
                       sprintf(buffer," %.10g",ddata);
                       break;
                     }
                   if (lpos+strlen(buffer) < 75U) /* 75U means unsigned */
                     {
                       fprintf(to, "%s", buffer);
                       lpos=lpos+strlen(buffer)+1;
                     }
                   else
                     {
                       offset--;
                       break;
                     }
                   if (offset != ncomps-1) fprintf(to, ",");
                 }
               if (offset != ncomps) fprintf(to, "...}\n"); else fprintf(to, "}\n");
               if (extraLen > 0)
                   fprintf(to, "[%s]\n", extraData);
            }
        }
    }
}       


/*+				S d s L i s t T o

 *  Function name:
      SdsListTo

 *  Function:
      List contents of an SDS object to a specified C file.

 *  Description:
      A listing of the contents of an SDS object is generated on
      the specified file.  The listing consists of the name type, dimensions
      and value of each object in the structure. The hierarchical structure
      is indicated by indenting the listing for components at each level. 
 
      For array objects only the values of the first few components are
      listed so that the listing for each object fits on a single line. 
      
 *  Language:
      C

 *  Declaration:
      void SdsListTo(FILE *to, SdsIdType id, StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) to        (FILE *) The file to write to.
      (>) id        (SdsIdType)  The identifier to the object to be listed
      (!) status    (long*) Modified status. SdsList calls a large number
                             of SDS routines so will return error status
                             values if an error occurs in any of these
                             routines.
                             


 *  Prior requirements:
      None.

 *  Support: Tony Farrell, {AAO}

 *  Version date: 23-Sep-2011
 *-
 */

void SdsListTo(FILE *to, SdsIdType id, StatusType * SDSCONST status)

{

    Sds__list(to, 0,id,"",status);
}



/*+				S d s L i s t

 *  Function name:
      SdsList

 *  Function:
      List contents of an SDS object

 *  Description:
      A listing of the contents of an SDS object is generated on
      standard output. The listing consists of the name type, dimensions
      and value of each object in the structure. The hierarchical structure
      is indicated by indenting the listing for components at each level. 
 
      For array objects only the values of the first few components are
      listed so that the listing for each object fits on a single line. 
      
 *  Language:
      C

 *  Declaration:
      void SdsList(SdsIdType id, StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  The identifier to the object to be listed
      (!) status    (long*) Modified status. SdsList calls a large number
                             of SDS routines so will return error status
                             values if an error occurs in any of these
                             routines.
                             


 *  Prior requirements:
      None.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 29-Apr-96
 *-
 */

void SdsList(SdsIdType id, StatusType * SDSCONST status)

{

    Sds__list(stdout, 0,id,"",status);
}



/*
 *+                                S d s F i n d B y P a t h
  
 * Function name:
      SdsFindByPath
  
 * Function:
        Accesses a structued Sds item using a path name to the item.
  
 * Description:
        This function is passed the id of an Sds structure and a
        name describing an element in that sturcture using a dot separated
        format.  It returns the id of the element.  For example, if we
        have a structure of the form

*v
*v	  Version              		Float  
*v	  FibreCentDist        		Float  
*v    	  PlateArray           		Struct
*v          Bundle             		Struct
*v            Fibres           		Struct
*v                xposition            	Float  
*v                yposition            	Float  
*v                transmission         	Float 
*v                bias                 	Float 
*v                broken               	Short 
*v
	Then the name "PlateArray.BundleArray.Fibres.xposition
	is a valid name.  In addition, structure array elements can
	be specified using  a specifiction like
	
*v
*v	item1[2]
*v	item2[2,3]
*v
	Where item1 is a one dimensional array and item2 is a two dimensional
	array. 	

	Note that the use of this routine requires that Sds names not use
	period or square brackets in their names.  This is not enforced any
	where so must be done by convention.  
  
 * Language:
      C
  
 * Call:
     (void) = SdsFindByPath (parent_id,name,id,status)
  
 * Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
  	(>)	parent_id	(SdsIdType) Identified of the structure
  	(>)	name		(char *) Structured name of the item to find
  	(<) 	id		(SdsIdType *) Identifier to the component
  	(!)	status		(long*)	Modified status.  Possible failure
  				codes are:
  				
  			SDS__NOMEM => Insufficent memory for creation
  			SDS__NOTSTRUCT => Parent id or subsequent 
  				non-terminating item is not a structure
  			SDS__NOITEM => No item with the specified name.
  			SDS__NOTARRAY => Attempt to access structure array
  				element in an item which is not a structure
  				array.
  
 * Include files: sds.h
  
  
 * Support: Tony Farrell, AAO
  
 *-
  
 * History:
      30-Apr-1996 - TJF - Original version
  
 */

/*
 *  Not all systems have strtok_r, so use an internal one.
 */

static char * Sds__Strtok_r(char * str2parse, SDSCONST char *delim, 
		       char **scanpoint );

/*
 *  Internal function used by SdsFindByPath.
 */

static void Sds__FindArrayElement(char * SDSCONST array_spec,
			     SDSCONST SdsIdType   StructId,
			     SdsIdType * SDSCONST ElementId, 
			     StatusType * SDSCONST status);


extern void SdsFindByPath (SdsIdType parent_id, 
			   SDSCONST char * name,
			   SdsIdType * id,
			   StatusType * SDSCONST status)
{
    
    char *copy;		/* Used for copy of name 	*/
    char *last;		/* Required by strtok_r 	*/
    
    char *component;	/* Component of name we are dealing with */

    if (*status != STATUS__OK) return;
    
/*
 *  Get a copy of name we can write to.
 */

    if ((copy = SdsMalloc(strlen(name)+1)) == 0)
    {
        *status = SDS__NOMEM;
        return;    
    }
    strcpy(copy,name);    
/*
 *  Get the first component name.  Copy the parent id and enter the loop
 */
    component = Sds__Strtok_r(copy,".",&last);
    *id = parent_id;
    while (component && (*status == STATUS__OK))
    {
        SdsIdType tid = 0;
    	char *array_spec = strchr(component, '[');
    	
/*
 *	If we have an array specification, then terminate the name
 *	and point to the begining of the array specification
 */
	if (array_spec)
	{
	    *array_spec = '\0';
	    ++array_spec;    
	}

/*
 *      Now "component" is the name we are dealing with. Find it.
 */
 	SdsFind(*id,component,&tid,status);
/*
 *	We are finished with the parent, but don't free the user supplied value.
 */
	if (*id != parent_id)
	{
	    StatusType ignore = STATUS__OK;
	    SdsFreeId(*id,&ignore);
	    *id = 0;
	}
/*
 *  	If we are dealing with an array specifiction, handle it by calling
 *  	FindArrayElement.
 */
	if (array_spec)
	{
	    SdsIdType ElemId=0;
	    StatusType ignore = STATUS__OK;
	    Sds__FindArrayElement(array_spec,tid,&ElemId,status);
	    SdsFreeId(tid,&ignore);
	    tid = ElemId;
	}
	

/*
 *      Success, we have found this component.  Free the parent's id
 *      (if it is not the supplied id), copy found id and get the next
 *      component.
 */
	
	if (*status == STATUS__OK)
	{
	    *id = tid;
            component = Sds__Strtok_r(NULL,".",&last);
	}
/*
 *      Failure after finding the item (array problem).  Tidy up.
 */
	
	else if (tid)
	{
	    StatusType ignore = STATUS__OK;
	    SdsFreeId(tid,&ignore);
	}
    }
/*
 *  Cases at this point
 *  	1.  Name is a empty string, loop fell though and parent_id = id
 *  	2.  Status ok, we have the item we are looking for in id
 *  	3.  Status bad.  Just return.
 */

    if ((*status == STATUS__OK)&&(*id == parent_id))
    {
        *id = 0;
        *status = SDS__NOITEM;
    }
    SdsFree((char *) copy);        
}


/*
 *  Sds__FindArrayElement
 *  
 *  Handle structure array specifications.  array_spec points to a specifiction
 *  of an array element in a structured array.  
 */
static void Sds__FindArrayElement(char * SDSCONST array_spec,
			     SDSCONST SdsIdType   StructId,
			     SdsIdType * SDSCONST ElementId, 
			     StatusType * SDSCONST status)
{
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    long ndims;
    SdsCodeType code;
    char name[SDS_C_NAMELEN];
    char *last;
    char *item;
    int spec_dims=0;
    
    if (*status != STATUS__OK) return;  

/*
 *  The item we are dealing with must be a structure array.  Check it
 */
    SdsInfo(StructId,name,&code,&ndims,dims,status);      
    if ((code != SDS_STRUCT)||(ndims < 1))
    {
        *status = SDS__NOTARRAY;
        return;
    }
    
/*
 *  Get the array element specification
 */
    item = Sds__Strtok_r(array_spec,",]",&last);    
    while ((item)&&(spec_dims < SDS_C_MAXARRAYDIMS))
    {
        dims[spec_dims] = atol(item);
        ++spec_dims;
        item = Sds__Strtok_r(NULL,",]",&last);    
    }
/*
 *  Find this cell.
 */
    
    SdsCell(StructId, spec_dims, dims, ElementId, status);
        
}


/*
 * Sds__Strtok_r.c
 * 
 * Tony Farrell, AAO
 * based on code of xstrtok by D'Arcy J.M. Cain and Henry Spencer
 *
 * If the delimiter is a space then leading whitespace is ignored and any
 * amount of whitespace is treated as a single delimiter.
 *
 */
#include	<ctype.h>

static char * Sds__Strtok_r(char * str2parse, SDSCONST char *delim, 
		       char **scanpoint )
{
	char		*scan;
	char		*tok;

	/* figure out what stage we are at */
	if (str2parse == NULL && *scanpoint == NULL)
		return(NULL);			/* last one went last time */

	if (str2parse != NULL)
	{
		scan = str2parse;	/* this is the first time */
	}
	else
		scan = *scanpoint;

	/* if delim is a space then skip leading whitespace */
	if (*delim == ' ')
		while (isspace((int)(*scan)))
			scan++;

	/* are we finished with the line? */
	if (*scan == '\0')
	{
		*scanpoint = NULL;
		return(NULL);
	}

	/* otherwise we now point to the next token */
	tok = scan;

	/* normal strtok semantics */
	for (; *scan && !strchr(delim, *scan); scan++)
		;

	if (*scan)
		*scan++ = 0;

	*scanpoint = scan;	/* point to next string */
	return(tok);				/* and return token */
}



/*+				S d s F i l l A r r a y

 *  Function name:
      SdsFillArray

 *  Function:
 	Fill out the contents of a structured array.

 *  Description:
 	This routine will fill out an array of structures item with
 	the copies of a specified struture.
      
 *  Language:
      C

 *  Declaration:
      void SdsFillArray(SdsIdType array_id, SdsIdType elem_id, StatusType *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) array_id  (SdsIdType)  The identifier to the array of structures
      				object to be filled
      (>) elem_id        (SdsIdType)  The identifier to the object, copies
      				of which are to be put into the array 
      				of structures.
      (!) status    (long*) Modified status. SdsFillArray calls a large number
                             of SDS routines so will return error status
                             values if an error occurs in any of these
                             routines.
                             

 *  Bugs:
 	Due to a missing Sds routine, the resulting structure may have one
 	level too deep.

 *  Prior requirements:
      None.

 *  Support: Tony Farrell, {AAO}

 *  Version date: 226-Sep-96
 *-
 */

void SdsFillArray(SdsIdType array_id, SdsIdType elem_id, 
		StatusType * SDSCONST status)

{
    
    char name[SDS_C_NAMELEN];
    SdsIdType type;
    long andims,ndims;
    unsigned long adims[SDS_C_MAXARRAYDIMS],dims[SDS_C_MAXARRAYDIMS];
    unsigned long elements,element;
    register int i;

    if (*status != SDS__OK) return;

/*
 *  Validate array of structures item. It must be a structure and an array.
 */    
    SdsInfo(array_id, name, &type, &andims, adims, status);
    if (*status == STATUS__OK) 
    {
        if (type != SDS_STRUCT)
            *status = SDS__NOTSTRUCT;
        else if (andims == 0)
            *status = SDS__NOTARRAY;
    }

/*
 *  Work out the number of elements in the array.
 */    
    elements = 1;
    for ( i= 0 ; i<andims ; i++) elements *= adims[i];
 

/*
 *  Validate item to be inserted.  It must be a structure.
 */
    SdsInfo(elem_id, name, &type, &ndims, dims, status);
    if ((*status == STATUS__OK)&&(type != SDS_STRUCT))
        *status = SDS__NOTSTRUCT;
        
    if (*status != SDS__OK) return;

    for (element = 1; (*status == STATUS__OK) && (element <= elements) ; 
    								++element)
    {
	SdsIdType copy_id;
/*
 *   	Create a copy of the element to be inserted.
 */
        SdsCopy(elem_id, &copy_id, status);
/*
 *      Do the insertion.
 */   
 	SdsInsertCell(array_id,1,&element,copy_id,status);
/*
 *   	Tidy up.
 */ 	
         SdsFreeId(copy_id,status);
    }    
}
/*+				 S d s T y p e T o S t r 

 *  Function name:
      SdsTypeToString

 *  Function:
 	Given an Sds Type Code, return a pointer to a string.

 *  Description:
 	Just simply looks up the code and returns a string pointer describing
 	the type referred to by the Sds Code.
 	
 	If code is invalid, then will return "SDS_INVALID" or
 	"invalid type" depending on mode.
      
 *  Language:
      C

 *  Declaration:
      const char *SdsTypeToStr(code,mode)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) SdsCodeType (code) The Sds code.
      (>) mode	    (mode)  IF true,then return things like "SDS_STRUCT".
      				If false, then return things link
      				"Int", "Struct" etc.
      (!) status    (StatusType*) Modified status. 

 *  Support: Tony Farrell, {AAO}

 *  Version date: 18-Apr-97
 *-
 */
SDSCONST char *SdsTypeToStr(SdsCodeType code, int mode)
{
    switch (code)
    {
        case SDS_STRUCT:
            return (mode ? "SDS_STRUCT" : "Struct");
        case SDS_BYTE:
            return (mode ? "SDS_BYTE" : "Byte");
        case SDS_CHAR:
            return (mode ? "SDS_CHAR" : "Char");
        case SDS_UBYTE:
            return (mode ? "SDS_UBYTE" : "Ubyte");
        case SDS_SHORT:
            return (mode ? "SDS_SHORT" : "Short");
        case SDS_USHORT:
            return (mode ? "SDS_USHORT" : "Ushort");
        case SDS_INT:
            return (mode ? "SDS_INT" : "Int");
        case SDS_UINT:
            return (mode ? "SDS_UINT" : "UInt");
        case SDS_I64:
            return (mode ? "SDS_I64" : "Int64");
        case SDS_UI64:
            return (mode ? "SDS_UI64" : "Uint64");
        case SDS_FLOAT:
            return (mode ? "SDS_FLOAT" : "Float");
        case SDS_DOUBLE:
            return (mode ? "SDS_DOUBLE" : "Double");
	default:
	    break;
    }
    return (mode ? "SDS_INVALID" : "invalid type");
}

/*
 *+                                S d s F i n d B y N a m e
  
 * Function name:
      SdsFindByName
  
 * Function:
        Accesses a descendent of a structured Sds item using the descendent's name only.
  
 * Description:

        This function is passed the id of an Sds structure and a name
        of a descendent element of that structure. It returns the id
        of the element.  For example, if we have a structure of the
        form

*v
*v	  Version              		Float  
*v	  FibreCentDist        		Float  
*v    	  PlateArray           		Struct
*v          Bundle             		Struct
*v            Fibres           		Struct
*v                xposition            	Float  
*v                yposition            	Float  
*v                transmission         	Float 
*v                bias                 	Float 
*v                broken               	Short 
*v
	Then the name "xposition" is a valid name and this routine will return
        an SdsId pointing at that element.

        The skip parameter can be used to select elements other than
        the first encountered in the search. Setting skip to something
        other than zero on entry will return the skip+1'th occurance
        of the element in the structure. Note that skip is modified as
        a result of this routine, and so a copy should be made if it
        is used for anything else.
  
 * Language:
      C
  
 * Call:
     (void) = SdsFindByName (parent_id, name, &skip, &id,status)
  
 * Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
  	(>)	parent_id	(SdsIdType) Identified of the structure
  	(>)	name		(char *) Structured name of the item to find
        (!)     skip            (unsigned int *) On entry, if this is > 0 the routine
                                    will bypass the first skip elements of the
                                    given name, and return the skip+1'th elemement.
                                    On exit, it will be zero or, if an error occurs,
                                    it will be decremented by the number of elements
                                    skipped. If it is a NULL pointer the first match
                                    will be returned.
  	(<) 	id		(SdsIdType *) Identifier to the component
  	(!)	status		(long*)	Modified status.  Possible failure
  				codes are:
  				
  			SDS__NOMEM => Insufficent memory for creation
  			SDS__NOITEM => No item with the specified name.
  
 * Include files: sds.h
  
  
 * Support: Nick Rees, JAC
  
 *-
  
 * History:
      31-Jul-2002 - NPR - Original version
  
 */

void SdsFindByName( SdsIdType parent_id, 
                    SDSCONST char * name,
                    unsigned int * skip,
                    SdsIdType * id,
                    StatusType * SDSCONST status )
{
    SdsIdType child_id;
    char child_name[SDS_C_NAMELEN];
    SdsCodeType code;
    long ndims;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    int found = 0;
    long int index;

    if ( *status != STATUS__OK ) return;

    index = 1;
    SdsIndex( parent_id, index, &child_id, status );

    while ( *status == STATUS__OK && !found )
    {
	SdsInfo( child_id, child_name, &code, &ndims, dims, status );

        /* Check to see if we have a match */
        if ( strcmp( name, child_name ) == 0 )
        {
            /* Test for skip passed as a NULL pointer as well as a variable */
            found = (skip == 0) || ( *skip == 0 );
            if ( found ) *id = child_id;
            else (*skip)--;
        }

        /* If it is an SDS structure, search down the structure */
        if ( !found )
        {
            if ( code == SDS_STRUCT )
            {
                if ( ndims == 0 )
                {
                    SdsFindByName( child_id, name, skip, id, status );
                    found = (*status == STATUS__OK);
                }
                else
                {
                    /* Structured array - we have to search down every element */
                    unsigned long dims1;
                    int i;
                    SdsIdType nid;

                    for(i=1;i<ndims;i++) dims[0] *= dims[i];

                    for ( dims1=1; dims1<= dims[0] && !found; dims1++ )
                    {
                        SdsCell(child_id,1,&dims1,&nid,status);

                        if (*status == STATUS__OK)
                        {
                            SdsInfo( nid, child_name, &code, &ndims, dims, status );
                            if ( code == SDS_STRUCT )
                            {
                                SdsFindByName( nid, name, skip, id, status );
                                found = (*status == STATUS__OK);
                            }
                            *status = STATUS__OK;
                            SdsFreeId( nid, status );
                        }
                        *status = STATUS__OK;
                    }
                }
            }

            *status = STATUS__OK;
            SdsFreeId( child_id, status );
        }

        index++;
        if (!found) SdsIndex( parent_id, index, &child_id, status );
    }
}



/*+			  	S d s C h e c k I n i t

 *  Function name:
      SdsCheckInit

 *  Function:
       Initialise a structure used to check for failures to run SdsFreeId().

 *  Description:
       SdsCheckInit() and SdsCheck() are usefull when checking for SDS ID leaks.
       What you should do is bracket a bunch of calls to SDS which are expected
       to not result in a net allocation of SDS ID's by SdsCheckInit() and SdsCheck().

       This function initialises the structure used by SdsCheck() to determine if a
       leak has occured since the last call.  By using different chkData variables, you
       can bracket muliple sets of SDS calls.

 *  Language:
      C

 *  Declaration:
         void SdsCheckInit(SdsCheckType *chkData, StatusType * SDSCONST status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) chkData   (SdsCheckType *) Check initialisation data is written here.
      (!) status    (StatusType*) Modified status. 

 *  Support: Tony Farrell, {AAO}
 
 *  Version date: 25-Sep-2008
 *-
 */


SDSEXTERN void SdsCheckInit(SdsCheckType *chkData, StatusType *status)
{
    if (*status != STATUS__OK) return;
    SdsGetIdInfo(
        &chkData->_init_last_id,
        &chkData->_init_current_array_size,
        &chkData->_init_unused_ids);
}


/*+			  	S d s C h e c k 

 *  Function name:
      SdsCheck

 *  Function:
       Check for failures to run SdsFreeId().

 *  Description:
       SdsCheckInit() and SdsCheck() are usefull when checking for SDS ID leaks.
       What you should do is bracket a bunch of calls to SDS which are expected
       to not result in a net allocation of SDS ID's by SdsCheckInit() and SdsCheck().

       Invoke this function with the chkData structure returned by a previous call
       to SdsCheckInit().  This function will then determine if there has been
       any net change in the number of SDS identifiers outstanding since that
       call to SdsCheckInit().

       If a problem occurs, then status is set bad and a message is output
       to stderr indicating details of the problem.

       Note - function checks for any net change in SDS ids - reduction or increase
        in number of ID's outstadning.  

 *  Language:
      C

 *  Declaration:
         void SdsCheck(char *mes, SdsCheckType *chkData, StatusType * SDSCONST status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) mes       (const char *) Used in a mesage output.  If non-zero, A message is output to
                                    stderr contianining this string.
      (!) chkData   (const SdsCheckType *) Check initialisation data checked against.
                                    The leakCount item in this structure will be updated
                                    with the number of SDS ID's which leaks.  This value
                                    will be negative if ID's were released.
      (!) status    (StatusType*) Modified status.  Possible failure
  				codes are:
  				
  			SDS__CHK_LEAK => The outstanding number of SDS id's has increased.
                        SDS__CHK_RELEASED => The outstanding num of SDS id's  dropped.

 *  Support: Tony Farrell, {AAO}
 
 *  Version date: 25-Sep-2008
 *-
 */


SDSEXTERN void SdsCheck(
    const char *mes,
    SdsCheckType *chkData, 
    StatusType *status)
{

    INT32 id_number;
    INT32 id_array_size;
    INT32 unused_ids;
    INT32 used_now;
    INT32 used_before;
    if (*status != STATUS__OK) return;

    SdsGetIdInfo(&id_number, &id_array_size, &unused_ids);
 
    used_now = id_number - unused_ids;

    used_before = chkData->_init_last_id - chkData->_init_unused_ids;


    chkData->leakCount = used_now - used_before;
    if (used_now == used_before)
        return;


    if (used_now > used_before)
    {
        if (mes)
            fprintf(stderr,"SdsCheck:%s:Leaked %d SDS IDs (now %d, before %d)\n",
                    mes, (used_now - used_before), used_now, used_before);
        
        *status = SDS__CHK_LEAK;
    }
    else 
    {
        if (mes)
            fprintf(stderr,"SdsCheck:%s:Released %d extra SDS IDs then expected (now %d, before %d)\n",
                    mes, (used_before - used_now), used_now, used_before);
        *status = SDS__CHK_RELEASED;
    }
}



/*+			  	S d s E r r o r T o S t r i n g

 *  Function name:
      SdsErrorToString

 *  Function:
      Convert an SDS error code to a string.

 *  Description:
      This simple routine returns a string equivalent to an SDS (or ARG) error code.  This
      allows simple stand-alone SDS programs to convert such codes without needing the
      "mess" (message generation) library.   Note that because this function must be
      updated by hande when codes are added, it may get out of sync.

 *  Language:
      C

 *  Declaration:
         cosnt char * SdsErrorCodeToString(StatusType status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) status    (StatusType*) Status to be converted.

 *  Support: Tony Farrell, {AAO}
 
 *  Version date: 25-Sep-2008
 *-
 */

SDSEXTERN SDSCONST char * SdsErrorCodeString(StatusType error)
{
    switch (error)
    {
    case SDS__BADID:
        return ("SDS:Illegal Identifier");
    case SDS__NOMEM:
        return ("SDS:Insufficient Memory");
    case SDS__NOTSTRUCT:
        return ("SDS:Object is not a structure");
    case SDS__NOITEM:
        return ("SDS:No such item");
    case SDS__NOTPRIM:
        return ("SDS:Object is not a primitive");
    case SDS__TOOLONG:
        return ("SDS:Data too long");
    case SDS__UNDEFINED:
        return ("SDS:Data value is undefined");
    case SDS__LONGNAME:
        return ("SDS:Object name is too long");
    case SDS__EXTRA:
        return ("SDS:Too much extra data");
    case SDS__INVCODE:
        return ("SDS:Invalid type cod");
    case SDS__INVDIMS:
        return ("SDS:Invalid dimensions");
    case SDS__NOTARRAY:
        return ("SDS:Not a structure array");
    case SDS__INDEXERR:
        return ("SDS:Indices invalid");
    case SDS__ILLDEL:
        return ("SDS:Object cannot be deleted");
    case SDS__NOTSDS:
        return ("SDS:Not a valid SDS object");
    case SDS__NOTTOP:
        return ("SDS:Not a top level object ");
    case SDS__EXTERN:
        return ("SDS:Illegal operation on external object");
    case SDS__NOTARR:
        return ("SDS:Object is not an array");
    case SDS__VERSION:
        return ("SDS:Object created by an incompatible SDS version");
    case SDS__FOPEN:
        return ("SDS:Error opening file");
    case SDS__FWRITE:
        return ("SDS:Error writing to a file");
    case SDS__FREAD:
        return ("SDS:Error reading a file");
    case SDS__TYPE:
        return ("SDS:Object has incorrect type");
    case SDS__TIMEOUT:
        return ("SDS:Timeout on semaphore access");
    case SDS__INVPRIMTYPE:
        return ("SDS:Invalid primitive type code");
    case SDS__INVSTRUCTDEF:
        return ("SDS:Invalid structure definition");
    case SDS__SYNTAX:
        return ("SDS:Parsing syntax error");
    case SDS__INVSTRUCTURE:
        return ("SDS:Invalid structure");
    case SDS__INVTYPETYPE:
        return ("SDS:Invalid typedef type");
    case SDS__STRUCTMULTDEF:
        return ("SDS:Multipled defined structure");
    case SDS__INVINT:
        return ("SDS:Invalid integer");
    case SDS__INVTYPE:
        return ("SDS:Invalid type");
    case SDS__INVINPUT:
        return ("SDS:Invalid input");
    case SDS__STRUCTARRAY:
        return ("SDS:Array of structures");
    case SDS__MAXDIMS:
        return ("SDS:Maximum number of dimensions");
    case SDS__NOINPUT:
        return ("SDS:Empty input file");
    case SDS__TCL_ARGERR:
        return ("SDS:Error in command arguments");
    case SDS__TCL_CMDERR:
        return ("SDS:Unspecified error in Tcl command");
    case SDS__TESTERR:
        return ("SDS:Test failure");
    case SDS__INVITEM:
        return ("SDS:Item definition does not match the definition expected by the appliaction code");
    case SDS__CHK_LEAK:
        return ("SdsCheck indicates a SDS data leak");
    case SDS__CHK_FREE:
        return ("SdsFreeIdAndCheck indicates SDS misuse (Missing SdsDelete() or SdsReadFree() call)");
    case SDS__CHK_RELEASED:
        return ("SdsCheck() indicates an unexpected release of an SDS id");
    case SDS__SEM_INIT_ERR:
        return ("SDS Semaphore/Mutex initialisation error");
    case SDS__SEM_TAKE_ERR:
        return ("Failed to take SDS Semaphore/Mutex");
    case SDS__AMBIGIOUS_TYPE:
        return ("Ambigous type name - when compling C to SDS structure");
    case  ARG__CNVERR:
        return ("ARG:Type Conversion Error");
    case  ARG__NOTSCALAR:
        return ("ARG:Object is not a scalar");
    case  ARG__NOTSTRING:
        return ("ARG:Object is not a strin");
    case  ARG__MALLOCERR:
        return ("ARG:Malloc failed");
    case STATUS__OK:
        return ("NO ERROR");
    default:
    {
        static char string[100];
        sprintf(string,"SdsErrorCodeString():Uknown error code (not SDS or ARG), code = %ld (%%x%lx)",
                (long)error,(long)error);
        return string;
    }
    } /* switch */
}
