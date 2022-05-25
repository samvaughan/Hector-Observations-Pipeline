#ifndef _SDSC_INCLUDE_
#define _SDSC_INCLUDE_ 1
/*
 *	Sds compiler include file
 * 
 *	Tony Farrell	15-Jul-1992
 *
 *  @(#) $Id: ACMM:sds/sdscompiler.h,v 3.94 09-Dec-2020 17:15:25+11 ks $
 *
 * History:
 *	17-Jul-1992 - TJF - Original version.
 *      03-Mar-1995 - TJF - SdsIntAsLong changed to SdsIntAs32Bit.  
 *			Changed comments for SdsOutputMessages.
 *			Removed error code definitions, now part of sds_err.
 *      06-Nov-1997 - TJf - Add SdsDelcareType, SdsVarName and 
 *                    SdsFunctionName variables.
 *      29-Jul-1998 - TJF - Remove DetailsType structure.  Put dimensions
 *                    and structure pointer within StructElementType and
 *                    TypeDefType.
 *      04-Apr-2018 - TJF - Add SdsIntCompatibility flag, which allows input
 *                     files to use "int", "long int" etc instead of INT32, INT64.
 *
 *
 */

/*
 *  The alpha OSF lexer generated unsigned char for the value of yytext 
 *  (this could be part of a move towards being eight bit clean).  Others 
 *  we have built on use plan char.   The VMS alpha is a bit of a problem
 *  since we actually generate the lexer source on the machine on which
 *  we keep the DRAMA source.  At the moment this in a SunOS 4.1 machine
 *  so we don't what unsigned char on it.
 */
#if defined(__alpha) && !defined(__VMS)

    typedef unsigned char LexChar;
#else
    typedef char LexChar;
#endif



typedef struct			/* The type LexString holds strings read  */
	{ char *str;		/* by lex.  It also keeps the line number */
	  int line_no;
	  char *filename;	/* Input file name at this point          */	
	} LexStringType;


#define NIL 0

#include "sds.h"		/* SDS standard include file		*/
#include "sds_err.h"		/* Sds error codes			*/

#define SDS_PRIM_MIN SDS_CHAR	/* Lowest Primitive code		*/
#define SDS_PRIM_MAX SDS_UI64	/* Lowest Primitive code		*/
#define SDS_TYPEDEF 100		/* Typedef codes, only used by SDSC	*/


#define MAXDIMS	7		/* Maximum number of dimensions	*/

/*
 *  The DimsType hold details of an array's dimensions.
 */
typedef struct {			
		int ndims;	/* 0 = not arrays, > 0 = num dimensions*/
		unsigned long  dims[MAXDIMS];/* The dimensions		*/
	} DimsType;

typedef struct StructDefStruct StructDefType ;/* Defines a structure	*/

/*
 *	A structure is said to consist of elements.  The details of
 *	each element is contained in this  structure as is a pointer
 *	to the next element.
 */
typedef struct StructElementStruct {	/* Struct Element details	*/
	struct StructElementStruct *next; /* Pointer to next element 	*/
	LexStringType name;		/* name	of element		*/
	int type;			/* type of element (SDS_?)	*/
        DimsType dims;		        /* Dimensions  	               */
        StructDefType *structPtr;	/* Struct defaults if struct	*/
	} StructElementType; 
/*
 *	This structure contains the complete details of a structure and
 *	points to the next structure on a list of structures
 */
struct  StructDefStruct {	/* Defines details of a structure */
        DimsType dims;          	/* Dimensions */
	StructElementType *first;	/* Pointer to the first element	 */
	struct StructDefStruct *next;	/* Next structure definition	*/
	LexStringType name;		/* Name of the structure	*/
	};
/*
 *	This structure contains the details of a typedef name and points
 *	to the next on the list.
 */
typedef struct TypeDefStruct {		/* Holds typedef information	*/
	struct TypeDefStruct *next; 	/* Next type def                */
	LexStringType name;		/* Typedef name			*/
	int type;			/* type of element (SDS_?)	*/
        DimsType dims;		        /* Dimensions  	               */
        StructDefType *structPtr;       /* Struct defaults if struct	*/
	} TypeDefType;

/*
 *	Routine prototypes.
 */
extern void Sds__makeString(LexChar *, LexStringType *string);
extern StructElementType * Sds__primitiveElement(int type, LexStringType *name, 
				int ndims, LexStringType dims[]);

extern StructElementType * Sds__structureElement(StructDefType *sd,
			        LexStringType *name,
			        int ndims, LexStringType dims[]);

extern StructElementType * Sds__typedefElement(TypeDefType *type, 
				LexStringType *name,				  				int ndims, LexStringType dims[]);
extern StructElementType * Sds__joinElementLists ( StructElementType *list1,
				       StructElementType *list2);

extern StructDefType * Sds__makeStruct (LexStringType *name,
		 StructElementType *list);

extern StructDefType * Sds__lookupStruct(LexStringType *name);

extern int Sds__generateStruct(StructDefType *s, LexStringType *name,
				int ndims, LexStringType dims[]);
extern void Sds__generateStructNamed();

extern void Sds__primitiveType (int type, LexStringType *name,
			        int ndims, LexStringType dims[]);
extern void Sds__structureType (StructDefType *s, LexStringType *name,
				int ndims, LexStringType dims[]);
extern void Sds__typeType ( LexStringType *n1, LexStringType *n2,
			        int ndims, LexStringType dims[]);
extern TypeDefType * Sds__lookupType (LexStringType *name);
extern StructDefType * Sds__lookupSType (LexStringType *name);

/*
 *	Global variables used by the compiler.
 */


extern StatusType SdsParsingError;/* Error code put here	*/
extern int SdsOutputMessages;   /* If SDS_COMP_MESS_ERROR, output error */
                                /* messages as well as setting status */
                                /* If SDS_COMP_MESS_WARN, also output */
                                /* warning messages                     */
extern int SdsOutputCode;	/* If true, Output code to generate structure */                                /* If false, create the structure	*/
extern int SdsIntCompatibility;  /* If true, then we allow the use of "int", "long int" etc */
extern SdsIdType SdsId;		/* If CreateStruct, the SdsId of the result */ 
extern char SdsFileName[];	/* Current file name			*/
extern int SdsInputFile;	/* If true, we are taking input from a file */
extern SDSCONST char *SdsInputString;	/* If InputFile is false, the string we
					are reading		*/ 
extern unsigned SdsDims;	/* Indicates the max number of array dimensions */
extern int SdsIntAs32Bit;	/* Treat ambiguous ints as long ints ? */

extern char * SdsFunctionName;  /* If set non-null, the name of the function
                                   to be created */
extern char * SdsDeclareType;   /* If set non-null, the name of the Sds type
                                   to declare (can be declared in file */
extern char * SdsVarName;       /* If non-null, the name of the variable 
                                   to be declared */
extern int SdsGenerateTcl;      /* True to generate Tcl instead of Sds */


#endif
