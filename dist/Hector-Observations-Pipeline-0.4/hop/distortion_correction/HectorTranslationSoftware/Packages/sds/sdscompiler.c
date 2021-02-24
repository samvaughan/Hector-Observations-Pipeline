/*	
 *	Self-defining Data System
 *
 *	C structure compiler.
 *
 *	Tony Farrell, 17-Jul-1992.
 * 
 * History:
 *     17-Jul-1992 - TJF - Original version.
 *     04-Sep-1992 - TJF - Use type SdsIdType for Sds id's variables output.
 *     07-Sep-1992 - JAB - Modifications to work under VxWorks.
 *     08-Jul-1993 - TJF - Change from VXWORKS macro to VxWorks
 *     15-Dec-1993 - TJF - Free Sds Id's when possible.
 *     06-Jan-1993 - TJF - Change various variables which hold sds id's
 *				from int to SdsIdType
 *     15-Apr-1994 - TJF - Include stdlib.h and string.h
 *     27-Oct-1994 - TJF - Introduce LexChar types characters taken from 
 *			   yytext in the lexer
 *     03-Feb-1995 - TJF - The messages argument to SdsCompiler and setting
 *			of SdsOutputMessages now can have two values, 
 *			SDS_COMP_MESS_ERROR and SDS_COMP_MESS_WARN.
 *			SdsInputString now defined as SDSCONST
 *	01-Aug-1996 - TJF - Support arrays of structures for output source mode
 *				(not supported for generate on fly mode as yet).
 *				Support 64 bit integers.
 *	26-Sep-1996 - TJF - The name of an anonymous structures at the top 
 *                      level is taken from the variable name (output source 
 *                      mode only as yet).  On fly mode now supports
 *			structure arrays.
 *      06-Nov-1997 - TJF - Support mode where function name is supplied
 *                      so that the output is self contained.   Support
 *                      mode where the variable to be declared and its name
 *                      can be supplied.  The routine genereate should
 *                      return SdsIdType not int.
 *      06-Nov-1997 - TJF - Support generating Tcl code (using commands
 *                      available in Dtcl from DRAMA V1.2).
 *      29-Jul-1998 - TJF - Add function Sds__linkedListRepair and use it
 *                      each time after using linkedList so that we can
 *                      use the circular list again.
 *
 *                      Add functions Sds__outLevelZeroFunc,
 *                      Sds__outLevelZeroNoFunc, Sds__outLevelNotZeroHead,
 *                      Sds__outCBlockVars, Sds__outLevelZeroHead,
 *                      Sds__outStructArrayNew, Sds__outStructNew and
 *                      Sds__outStructArrayFill which are used to make
 *                      the rather large Sds__outSdsStruct far more readable.
 *
 *                      Fix problems with output of structured SDS arrays.
 *                      Ensure when compiling rather then generating the
 *                      output, that we complain about names being too long.
 *      21-Jan-2002 - TJF - Fix compiler warnings with stricter GCC
 *                      warnings level.
 *      05-Apr-2004 - TJF - Rename yyparse to SdsYyparse(). 
 *       
 *
 *
 *
 * The C structure compiler is uses Lex and Yacc to generate the
 * lexical analyzer and parser.  This file contains the SdsCompiler routine 
 * and all the action routines called by yyparse (in the Yacc output - but
 * renamed to SdsYyparse()) by the Makefile.
 * when the various constructs are recongnized.  The routines herein build
 * up two lists, one of typedefs (Types) and the other of structure definitions
 * (Structures). The routine Sds__generateStruct is called when a structure 
 * is declared too generate the actual structure. 
 *  
 *  
 * A structure definition (StructDefType) contains the structure name and
 * a list describing the elements (type StructElementType).  This list consists 
 * of a singly linked list of nodes.  The last element points to the first 
 * (so they are linked cyclically).  A complete list is represented by a 
 * pointer to its last element.  This means it is easy to add to the
 * end of the list and to find the beginning of the list.  Empty lists are 
 * represented by a nil pointer.  New nodes are created pointing to themselves.
 * When constructed, the structure definition is stored in a lookup table
 * (Structures) for later use.  The following routines are used to build
 * and lookup the structures -
 *
 *	Sds__primitiveElement -> Makes a node defining a primitive (not a
 *				 structure).
 *	Sds__structureElement -> Makes a node defining a structure within
 *				 a structure.
 *	Sds__typeElement      -> Makes a node defining a typedef type.
 *   	Sds__joinElementList  -> Join two element lists
 *      Sds__makeStruct	      -> Create the structure definition and
 *			         save it on the lookup list.
 *	Sds__lookupStruct     -> Lookup a structure by name
 *
 * A typedef (TypeDefType) is an alias for a type.  We maintain a list of
 * types.  The following routines are used to build an lookup typedefs -
 *
 *      Sds__primitiveType    -> Makes a typedef for a primitive type.
 *      Sds__structureType    -> Makes a typedef for a structure.
 *      Sds__typeType	      -> Makes a typedef for a typedef name.
 *      Sds__lookupType	      -> Lookup a typedef.
 *      Sds__lookupSType      -> Lookup a typedef, but it must be a
 *				structure and only structures can be
 *				returned.
 *
 *
 * The other externally called routines in this module are-
 *
 *      Sds__makeString	      -> Saves a string in a LexString structure.
 *	Sds__generateStruct   -> Generates an Sds version of a structure.
 *				 (either by outputting code or call SdsNew).
 *	SdsCompiler	      -> User callable interface to the compiler.
 *
 *
 * Internal routines -
 *	Sds__outSdsStructure  -> Output code to generate an Sds version
 *				 of the supplied structure.  The code is
 *				 output to stdout.
 *      Sds__outStructElem    -> Output code to generate a list of Sds
 *				 structure elements.
 *	Sds__outArrayDims    -> Output code to generate an array dimensions
 *      Sds__createSdsStruct  -> Create an Sds version of the supplied
 *				 structure.  The sds id of the structure is
 *				 returned.
 *      Sds__createStructElem -> Create the elements of an Sds structure.
 *      Sds__linkedList	      -> Convert a circular linked list of structure
 *				 elements to a non-circular linked list.
 *      Sds__linkedListRepair -> Convert the linked list back to a circular
 *                               structure.
 *	Sds__makeDims	      -> Convert dimensions from strings to the
 *				 appropriate form
 *
 *      Sds__outLevelZeroFunc -> If we have a function name, output
 *                               function name and header.
 *      Sds__outLevelZeroNoFunc -> If we don't have a function name, output
 *                               the header.
 *      Sds__outLevelNotZeroHead -> Called to output the header for a 
 *                                non-zero level structure.
 *      Sds__outCBlockVars    -> Output variables for a structure block.
 *      Sds__outLevelZeroHead -> Output a level zero structure block header.
 *      Sds__outStructArrayNew -> Output SdsNew for a structure array.
 *      Sds__outStructNew     -> Output a SdsNew call for a non-array structure
 *      Sds__outStructArrayFill -> Fill a structure array.
 */
/*
 * Include files
 */


static const char *rcsId="@(#) $Id: ACMM:sds/sdscompiler.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include "status.h"
#include "sdscompiler.h"
#ifdef VxWorks
#include <stdioLib.h>
#else
#include <stdio.h>
#include <time.h>
#endif
#include <stdlib.h>
#include <string.h>

/*
 * this should be in sds.h, which is included by sdscompiler.h, but was
 * added late in the process.
 */
#ifndef SDS_C_NAMELEN
#define SDS_C_NAMELEN 16
#endif

/*
 * Global variables, these definitions should match those in sdscompiler.h
 */
StatusType SdsParsingError;	/* Any error during parsing results in a */
				/* non zero code here			 */
int SdsOutputMessages;		/* True to output error messages	 */
int SdsOutputCode;		/* True to output code instead of	 */
 				/* creating an sds structure		 */
int SdsIntCompatibility = 0;     /* Do we allow int and long int etc?     */
SdsIdType SdsId;			/* If OutputCode is false, the SdsId	 */
char SdsFileName[200];		/* Current input file			 */
int SdsInputFile = 1;		/* If true, input is a file		 */
char SDSCONST *SdsInputString = "";	/* If SdsInputFile is false, the input string */
unsigned SdsDims = 0;		/* If non-zero, indicates the maximum    */
				/* number of dimensions of any array	*/
int SdsIntAs32Bit = 1;		/* Treat ambiguous ints as 32 bit ints 	*/

char *SdsFunctionName = NIL;    /* If non-null, we output a file which include
                                   a full function definition, instead of
                                   one with just the body.  This will contain
                                   the name of the file                 */
char *SdsDeclareType = NIL;     /* If no variable is declared in the parsed
                                   file, then this is the type of the variable
                                   to declare                           */
char *SdsVarName = NIL;         /* If no variable is declared in the parsed
                                   file, then this is the name of the variable
                                   to declare                           */
int SdsGenerateTcl = 0;         /* If true, genereate Tcl instead of C */
/*
 * Module only variables
 *
 * This is a list which will contain of the structures defined so far;
 */
static StructDefType *Structures = NIL;
/*
 * This is a list which will contain of the user types defined so far;
 */
static TypeDefType *Types = NIL;
/*
 * This array maps Sds codes to character strings
 */
static char *SdsCodes[] = {
	"",
	"SDS_CHAR",
	"SDS_UBYTE",
	"SDS_BYTE",
	"SDS_USHORT",	
	"SDS_SHORT",
	"SDS_UINT",
	"SDS_INT",
	"SDS_FLOAT",
	"SDS_DOUBLE",
	"Invalid Type",
	"SDS_I64",
	"SDS_UI64"} ;

static char *TclSdsCodes[] = {
	"",
	"CHAR",
	"UBYTE",
	"BYTE",
	"USHORT",	
	"SHORT",
	"UINT",
	"INT",
	"FLOAT",
	"DOUBLE",
	"Invalid Type",
	"I64",
	"UI64"} ;
/*
 *  This indent macro writes a number of spaces { (arg+1)*4 } to the standard
 *  output.  It is used to ensure the code which is output is indented
 *  correctly.
 */
#define indent(A) { register unsigned i; for (i = 0 ; (i < ((unsigned)(A)+1)*4) ; ++i) putchar(' ');}

/*
 *  Function prototypes.  Any function not prototyped in includes files should
 *  be prototyped here.
 */
static void Sds__outSdsStruct(StructDefType *s, LexStringType *name, 
                              int level, DimsType *dims,
                              StatusType *status);
static void Sds__outStructElem(StructElementType *elem, int level, 
			StatusType *status);
static void Sds__outArrayDims(int level,
		      DimsType *dims,
		      StatusType *status);
static SdsIdType Sds__createSdsStruct(StructDefType *s, LexStringType *name, 
                                      SdsIdType id, DimsType *dims, 
                                      StatusType *status);
static void Sds__createStructElem(StructElementType *elem, SdsIdType topid, 
                	StatusType *status);
static StructElementType * Sds__linkedList( StructElementType *c);

void SdsYyparse(void);

#ifdef SUN
    int printf(char *,...);
    int fprintf(FILE *, char *, ...);
    int fflush(FILE *);
    int sscanf(char *, char *, ...);
    int _flsbuf(unsigned char, FILE *);
    time_t time(time_t *);
#endif
#ifdef DECSTATION
    void * malloc(int);
#endif
/*
 *
 * Sds__makeString 
 */

extern void Sds__makeString(LexChar *s, LexStringType *string)
{
/*
 *  Since the space a string is read into by lex is overwritten, we must
 *  malloc some space for strings.  At the same time, we save the current
 *  filename and line number so they can be used in messages related to this
 *  string.
 *
 *  Create space for and save the string
 */
    extern int SdsYylineno;
    string->str = (char *)malloc(strlen((char *)s)+1);
    strcpy(string->str, (char *)s);
/*
 *  Do the save of the current program name and save the line number
 */
    string->filename = (char *)malloc(strlen(SdsFileName)+1);
    strcpy(string->filename, SdsFileName);
    string->line_no = SdsYylineno; 

}
/*
 *
 *  Setup a dimensions array.  dimensions->ndims should have the number of
 *  of valid dimensions already in the array.  New dimensions will be
 *  added at the end.
 */
static void Sds__makeDims ( int ndims, LexStringType dims[], DimsType *dimensions)
{
    register unsigned j = dimensions->ndims;
    if (ndims >= 0)
    {
        register unsigned i;
        for (i = 0 ; (i < (unsigned)ndims)&&(j < MAXDIMS) ; ++i, ++j)
        {
	    if (sscanf(dims[i].str,"%li",&dimensions->dims[j]) != 1)
	    {
	        if (SdsOutputMessages)
		{
		   fprintf(stderr,"%s:%d:Error converting %s into an integer.\n"
                           ,dims[i].filename, dims[i].line_no, dims[i].str);
	           if (!SdsParsingError)
				SdsParsingError = SDS__INVINT;
	        }
	    }
	}
	if (i < (unsigned)ndims)
	{
	    if (SdsOutputMessages)
    	    {	
	        fprintf(stderr,"%s:%d:Exceeded maximum number of dimensions\n"
                           ,dims[i].filename, dims[i].line_no );
	        if (!SdsParsingError)
		    SdsParsingError = SDS__MAXDIMS;
            }
	}
	dimensions->ndims = j;
 	if (j > SdsDims)
	    SdsDims = j; /* Save number of dimensions */
    }
}
/*
 *
 *  Sds__primitiveElement - Makes a node definig a primitive structure
 *			    element.
 */

extern StructElementType * Sds__primitiveElement ( 
	int type,			/* Type of element		*/
	LexStringType *name,		/* Element name			*/
	int ndims,			/* Number of dimensions		*/
	LexStringType dims[])		/* Dimensions			*/
{
	StructElementType *s = NIL;
	if ((type < SDS_PRIM_MIN)||(type > SDS_PRIM_MAX))
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Invalid primitive type, item \"%s\"\n",
  			             name->filename,name->line_no,name->str);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__INVPRIMTYPE;
        }
	else
	{
#	    ifdef DEBUG
                printf("Making primitive element %s\n",name->str);
#	    endif
	    s = (StructElementType *) malloc(sizeof(StructElementType));
	    s->next = s;		/* Created pointing to itself	*/
	    s->name = *name;		/* The name of this primitive	*/
	    s->type = type;		/* Its SDS type			*/
	    s->dims.ndims = 0;	        /* Initialise dimensions to none */
	    Sds__makeDims(ndims,dims,&s->dims);
	
	}
	return(s);
}
 
/*
 *
 * Sds__structureElement - Makes a node defining a structure within a 
 *			    structure.
 */
extern StructElementType * Sds__structureElement ( 
	StructDefType *sd,		/* Structure definition		*/
	LexStringType *name,		/* Element name			*/
	int ndims,			/* Number of dimensions		*/
	LexStringType dims[])		/* Dimensions			*/
{
	StructElementType *s = NIL;
	if (sd == NIL)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Invalid structure item \"%s\"\n",
			name->filename,name->line_no,name->str);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__INVSTRUCTURE;
        }
	else
	{
#	    ifdef DEBUG
                printf("Making structure element %s\n",name->str);
#	    endif
	    s = (StructElementType *) malloc(sizeof(StructElementType));
	    s->next = s;	    /* Created pointing to itself	*/
	    s->name = *name;	    /* Save the name			*/
	    s->type = SDS_STRUCT;   /* SDS type				*/
	    s->structPtr = sd;      /* Structure details		*/
	    s->dims = sd->dims;     /* Initial dims is of base type     */
            /* Add dimensions added at this stage */
	    Sds__makeDims(ndims,dims,&s->dims);

	}
	return(s);
}
 
/*
 *
 *  Sds__typedefElement - Makes a node defining an element with a typedef
 *			name as the type.  (It could be a primitve, a
 *			primitive array or a structure/structure array).
 */
StructElementType * Sds__typedefElement ( 
	TypeDefType *t,		/* Type definition		*/
	LexStringType *name,	/* Element name			*/
	int ndims,		/* Number of dimensions		*/
	LexStringType dims[])	/* Dimensions			*/
{
	StructElementType *s = NIL;
	if (t == NIL)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Invalid type name\n", 
                                           name->filename,name->line_no);
	    }
	    if (!SdsParsingError)
		    SdsParsingError = SDS__INVTYPETYPE;
        }
	else
	{
#	    ifdef DEBUG
                printf("Making typedef element %s\n",name->str);
#	    endif
	    s = (StructElementType *) malloc(sizeof(StructElementType));
	    s->next = s;	    /* Created pointing to itself   */
	    s->name = *name;	    /* Element name		    */
	    s->type = t->type;	    /* The sds type		    */
            s->dims = t->dims;      /* The dimensions               */
            s->structPtr = t->structPtr; /* Pointer to struture def */
            /* Add dimensions recorded at this stage */
            Sds__makeDims(ndims,dims,&s->dims);
		
	}
	return(s);
}

/*
 *
 *  Sds__joinElementLists - Join two element lists
 */
StructElementType * Sds__joinElementLists (
	StructElementType *list1,		/* Pointer to list 1	*/
	StructElementType *list2)		/* Pointer to list 2	*/
{

	StructElementType *tmp = NIL;
#	ifdef DEBUG
		printf("Joining two structure element lists\n");
#	endif
	if (list2 == NIL)
	{
	    list2 = list1;
	    list1 = NIL;
	}
	/* Assert, if list1 is not NIL, neither is list2	*/	
	if (list1 != NIL)
	{
	    tmp = list2->next;
	    list2->next = list1->next;
	    list1->next = tmp;
	}
	/* else, list2 points to the result	*/

	return(list2);
}
/*

 *  Sds__makeStruct - Takes an element list and a structure name and returns
 *	    the structure, after puting it on the lookup table.
 */
StructDefType * Sds__makeStruct( LexStringType *name,/* Name of structure*/
		StructElementType *list)	/* Elements		*/

{
	static int unique = 0;
	StructDefType *s = NIL;
	if (name) 
		s = Sds__lookupStruct(name);
	if (list == NIL)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Invalid structure definition - \"%s\"\n",
			 name->filename,name->line_no,name->str);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__INVSTRUCTDEF;
        }
	else if (s != NIL)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Struct \"%s\" multiply defined\n", 
				name->filename,name->line_no,name->str);
		fprintf(stderr,"%s:%d:Previous definition\n",
                                  s->name.filename, s->name.line_no);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__STRUCTMULTDEF;
	    s = NIL;
        }
	else
	{   
#	    ifdef DEBUG
		printf("Making structure %s, line %d\n",name->str,
                                                        name->line_no);
#           endif
	    s = (StructDefType *)malloc(sizeof(StructDefType));
            s->dims.ndims = 0;
	    s->first = list;		/* Point to element list    */
	    if (name != 0)		/* If a name is supplied use it */
	        s->name = *name;
            else		/* Otherwise, create an anonymous name */
	    {			
		LexChar n[20];
		++unique;
		sprintf((char *)n,"_anon%04d",unique);
		Sds__makeString(n,&s->name);
  	    }
/*
 *	    Put this structure in the list of structures
 */
	    s->next = Structures;
	    Structures = s;	
	}
	return(s);
}
/*
 *
 *  Sds__lookupStruct - Find a predefined structure by name.
 */
StructDefType * Sds__lookupStruct( LexStringType *name)
{
    StructDefType *s = Structures;
    while (s != NIL)
    {
	if (strcmp(s->name.str,name->str) == 0)
	    break;
	else
	    s = s->next;
   }
   return(s);
}
/*
 *
 *  Sds__primitiveType - Make a typedef name for a primitive type.
 */
 
void Sds__primitiveType ( 
	int type,			/* Type of element		*/
	LexStringType *name,		/* Type name			*/
	int ndims,			/* Number of dimensions		*/
	LexStringType dims[])		/* Dimensions			*/

{
	TypeDefType *t = Sds__lookupType(name);
	if (type < SDS_PRIM_MIN)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Invalid primitive type, item \"%s\"\n",
			name->filename,name->line_no,name->str);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__INVPRIMTYPE;
        }
	else if (t != NIL)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Attempt to redefine type %s\n",
                        name->filename,name->line_no,name->str);
		fprintf(stderr,"%s:%d:Previous definition of %s\n",
			t->name.filename,t->name.line_no,name->str);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__INVPRIMTYPE;
	    t = NIL;

	}

	else
	{
#	    ifdef DEBUG
                printf("Making primitive typedef %s\n",name->str);
#	    endif
	    t = (TypeDefType *) malloc(sizeof(TypeDefType));
	    t->next = Types;
	    Types = t;
	    t->name = *name;
	    t->type = type;
	    t->dims.ndims = 0;	/* Indicates base has no dimensions */
	    Sds__makeDims(ndims,dims,&t->dims);
	}
}

/*
 *
 *  Sds__structureType - Make a typedef name for a structure type.
 */
void Sds__structureType ( 
	StructDefType *sd,		/* Structure definition		*/
	LexStringType *name,		/* Element name			*/
	int ndims,			/* Number of dimensions		*/
	LexStringType dims[])		/* Dimensions			*/
{
	TypeDefType *t = Sds__lookupType(name);
	if (sd == NIL)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Invalid structure item \"%s\"\n",
			name->filename,name->line_no,name->str);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__INVPRIMTYPE;
        }
	else if (t != NIL)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Attempt to redefine type %s\n",
                        name->filename,name->line_no,name->str);
		fprintf(stderr,"%s:%d:Previous definition of %s\n",
			t->name.filename,t->name.line_no,name->str);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__INVPRIMTYPE;
	    t = NIL;

	}
	else
	{
#	    ifdef DEBUG
                printf("Making structure typedef %s\n",name->str);
#	    endif
	    t = (TypeDefType *) malloc(sizeof(TypeDefType));
	    t->next = Types;
	    Types = t;
	    t->name = *name;
	    t->type = SDS_STRUCT;
	    t->structPtr = sd;
            t->dims.ndims = 0;   /* Initliase dimensions to zero        */
	    sd->dims.ndims = 0;	/* Indicates the base type has no dims ??? */
	    Sds__makeDims(ndims,dims,&sd->dims);


	}
}
/*

 *  Sds__typeType - Make a new typedef name for a typedef name.
 */
void Sds__typeType ( 
	LexStringType *n1,		/* Old type			*/
	LexStringType *n2,		/* New type			*/
	int	ndims,			/* Number of dimensions		*/
	LexStringType dims[])		/* Dimensions			*/
{
	TypeDefType *t1 = Sds__lookupType(n1);
	TypeDefType *t2 = Sds__lookupType(n2);
	if (t1 == NIL)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Invalid type name \"%s\"\n",
			n2->filename,n2->line_no,n1->str);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__INVPRIMTYPE;
	    t2 = NIL;
        }
	else if (t2 != NIL)
	{
	    if (SdsOutputMessages)
	    {
	        fflush(stdout);	/* To align stdout with stderr	*/
	        fprintf(stderr,"%s:%d:Attempt to redefine type %s\n",
                        n2->filename,n2->line_no,n2->str);
		fprintf(stderr,"%s:%d:Previous definition of %s\n",
			t2->name.filename,t2->name.line_no,t2->name.str);
	    }
	    if (!SdsParsingError)
	        SdsParsingError = SDS__INVPRIMTYPE;

	}
	else
	{
#	    ifdef DEBUG
                printf("Making type typedef %s\n",n2->str);
#	    endif
	    t2 = (TypeDefType *) malloc(sizeof(TypeDefType));
	    *t2 = *t1;
	    t2->next = Types;
	    Types = t2;
	    t2->name = *n2;
/*
 *	    If the base type is not a structure, save any array dimensions
 */
	    if (t2->type != SDS_STRUCT)
	        Sds__makeDims(ndims,dims,&t2->dims);
            else if (ndims != 0)
	        Sds__makeDims(ndims,dims,&t2->structPtr->dims);
	}
}
/*

 * Sds__lookupType - Lookup a typedef name.
 */
TypeDefType * Sds__lookupType( LexStringType *name)
{
    TypeDefType *t = Types;
    while (t != NIL)
    {
	if (strcmp(t->name.str,name->str) == 0)
	    break;
	else
	    t = t->next;
   }
   return(t);
}
/*

 * Sds__lookupSType - Lookup a typedef name that should be a structure
 */
StructDefType * Sds__lookupSType( LexStringType *name)
{
    TypeDefType *t = Types;
    while (t != NIL)
    {
	if (strcmp(t->name.str,name->str) == 0)
        {
            if (t->type == SDS_STRUCT)
                return t->structPtr;
            else
            {
                fprintf(stderr,"Invalid type name \"%s\" (name found, but is NOT typedef of structure), line %d\n", name->str, name->line_no);
	        SdsParsingError = SDS__INVSTRUCTURE;
                return NIL;
            }
        }
        t = t->next;
   }
   SdsParsingError = SDS__INVSTRUCTURE;
   fprintf(stderr,"Invalid type name \"%s\", line %d\n    (should be name of typedef of structure - or maybe you are missing \"struct\" keyword)\n", name->str, name->line_no);
   return(NIL);
}
/*
 *
 *  Sds__generateStruct - Make a structure.
 */ 


extern int Sds__generateStruct(StructDefType *s, 
	LexStringType *name,		/* Element name			*/
	int ndims,			/* Number of dimensions		*/
	LexStringType dims[])		/* Dimensions			*/
{
    DimsType theDims;
    if (!s)
    {
        fprintf(stderr,"Unable to generate structure \"%s\", line %d\n", 
                name->str, name->line_no);
        return 0;
    }
/*
 *  Add extra dimensions required in the final structure.
 */
    theDims = s->dims;
    Sds__makeDims(ndims,dims,&theDims);
    
/* 
 * The SdsOutputCode variable determines if code is output or if the 
 * structure is created.  We can't do both since the list is damaged
 */
    SdsId = 0;
#ifndef VxWorks
    if (SdsOutputCode)
	Sds__outSdsStruct(s,name,0,&theDims,&SdsParsingError);
    else  
#endif
	SdsId = Sds__createSdsStruct(s,name,0,&theDims,&SdsParsingError);
     return(SdsId);
   
}

/*
 *  This function is called if the file contains only defintions, with no
 *  declaration.  If the SdsDeclareType variable has been set, then we
 *  can at as if that variable has been declared.  Otherwise, we
 *  generate an error.
 */
extern void Sds__generateStructNamed()
{
    if (!SdsDeclareType)
    {
        if (SdsOutputMessages)
              fflush(stdout); /* To align stdout with stderr */
        fprintf(stderr,"%s:Parsing failure - No declaration\n",
                                                     SdsFileName);
        if (!SdsParsingError)
            SdsParsingError = SDS__SYNTAX;
    }
    else
    {
        LexStringType declareType;
        StructDefType *SType;
        Sds__makeString((LexChar *)SdsDeclareType,&declareType);
        SType = Sds__lookupSType(&declareType);

        if (!SType)
        {
            if (SdsOutputMessages)
                fflush(stdout); /* To align stdout with stderr */
            fprintf(stderr,"%s:Parsing failure - Invalid type name %s\n",
                    SdsFileName,SdsDeclareType);

            if (!SdsParsingError)
                SdsParsingError = SDS__INVTYPETYPE;

        }
        else if (SdsVarName)
        {
            LexStringType varName;
            Sds__makeString((LexChar *)SdsVarName,&varName);
            Sds__generateStruct(SType,&varName,0,0);
        }
        else
            Sds__generateStruct(SType,&declareType,0,0);
    }
 
}

/*
 
 * Sds__linkedList -  A StructElementType list is created as a circularly 
 *  linked list. We need just a normal linked list.  This routine does the 
 *  conversion.
 */
static StructElementType * Sds__linkedList( StructElementType *c)
{
	if (c != NIL)
	{
	    StructElementType *l = c->next;
	    c->next = NIL;
	    return(l);
	}
	else
	    return(NIL);
}
/*
 
 * Sds__linkedListRepair -  A StructElementType list is created as a 
 * circularly  linked list.  The above routine (Sds__linkedList) breaks it 
 * into a  normal linked list.  This routine makes it a linked list again.
 */
static void Sds__linkedListRepair( StructElementType *c,
                                   StructElementType *l)
{
    if (c != NIL)
        c->next = l;
}

#ifndef VxWorks
/*
 *  Sds__outLevelZeroFunc
 *
 *  Called if we have function name and we are at level zero.  Outputs
 *  the function header.
 */
static void Sds__outLevelZeroFunc(StructDefType *s, StatusType *status)
{

#ifndef macintosh
    time_t t = time(NULL);
#endif
    if (*status != STATUS__OK) return;

    if (SdsGenerateTcl)
    {
        printf("# This is a routine for defining a SDS structure ");
        printf("based on a C language\n# declaration.  ");
        printf("It has been generated by the SDS compiler.");
        printf("\n# The outer level structure is of type ");
        printf("\"%s\" (%s:%d)\n#\n",s->name.str,
               s->name.filename,
               s->name.line_no);
#ifndef macintosh
        printf("# Generated at %s\n",ctime(&t));
#endif

        printf("proc %s {} {\n",SdsFunctionName);
    }
    else
    {
        printf("/*  \n * This is a routine for defining a SDS structure");
        printf(" based on a C language\n * declaration.  ");
        printf("It has been generated by the SDS compiler.");
        printf("\n * The outer level structure is of type ");
        printf("\"%s\" (%s:%d)\n *\n",s->name.str,
               s->name.filename,
               s->name.line_no);
#ifndef macintosh
        printf(" * Generated at %s */\n",ctime(&t));
#endif

        printf("#define NULL 0\n");
        printf("#include \"sds.h\"\n");
        printf("extern SdsIdType %s (StatusType *status)\n",SdsFunctionName);
    }
}

/*
 *  Sds__outLevelZeroNoFunc
 *
 *  Called if we don't have function name and we are at level zero.  Outputs
 *  the function header.
 *  
 */
static void Sds__outLevelZeroNoFunc(StructDefType *s, StatusType *status)
{

#ifndef macintosh
    time_t t = time(NULL);
#endif

    if (*status != STATUS__OK) return;

    if (SdsGenerateTcl)
    {
        printf("# This is a routine for defining a SDS structure");
        printf(" based on a C language\n# declaration.  ");
        printf("It has been generated by the SDS compiler and should");
        printf(" be\n# included in a file which provides the routine");
        printf(" name using a declaration of\n# the form - \"");
        printf("proc routine {} {\"\n#\n");
        printf("# The outer level structure is of type ");
        printf("\"%s\" (%s:%d)\n# \n",s->name.str,
               s->name.filename,s->name.line_no);
#ifndef macintosh
        printf("# Generated at %s\n",ctime(&t));
#endif
    }
    else
    {
        printf("/*  \n * This is a routine for defining a SDS structure");
        printf(" based on a C language\n * declaration.  ");
        printf("It has been generated by the SDS compiler and should");
        printf(" be\n * included in a file which provides the routine");
        printf(" name using a declaration of\n * the form - \"SdsIdType routine");
        printf(" (StatusType *status)\"\n *\n");
        printf(" * The outer level structure is of type ");
        printf("\"%s\" (%s:%d)\n *\n",s->name.str,
               s->name.filename,s->name.line_no);
#ifndef macintosh
        printf(" * Generated at %s */\n",ctime(&t));
#endif
    }
}

/*
 *  Sds__outLevelNotZeroHead
 *
 *  Called to output the header for a non-zero level structure.
 */
static void Sds__outLevelNotZeroHead(
    int array,
    StructDefType *s, 
    LexStringType *name, 
    int level, 
    StatusType *status)
{
    if (*status != STATUS__OK) return;
    if (SdsGenerateTcl)
    {
        indent(level);
        printf("# Definition of structure %s\"%s\" of type \"%s\" at \
level %d (%s:%d)\n",(array ? "array " : ""),name->str, 
               s->name.str,level, s->name.filename,s->name.line_no);
    }
    else
    {
        indent(level);
        printf("/* Definition of structure %s\"%s\" of type \"%s\" at \
level %d (%s:%d)*/\n",(array ? "array " : ""),name->str, 
               s->name.str,level, s->name.filename,s->name.line_no);
    }

}
/*
 *  Sds__outCBlockVars
 *
 *  Output variables for a structure block.
 */
static void Sds__outCBlockVars(int level, int ndims, StatusType *status)
{
    if (*status != STATUS__OK) return;
    indent(level);
    printf("SdsIdType tid%d = 0; /* SDS id for structure*/\n",level);
            
    if (ndims)
    {
        indent(level);
        printf(
            "SdsIdType tid%d = 0; /* SDS id for structure array elem*/\n",
            level+1);
    }

}
/*
 *  Sds__outLevelZeroHead
 *
 *  Output a level zero structure block header.  
 */
static void Sds__outLevelZeroHead(StatusType *status)
{
    if (*status != STATUS__OK) return;
    if (SdsDims)
    {
        indent(0);
        printf(
           "unsigned long dims[%d]; /* Dimension array for array elements*/\n",
           SdsDims);
    }
    indent(0);
    printf("SdsIdType id = 0; /* Primitive element sds id */\n");
    indent(0);
    printf("if (*status != STATUS__OK) return(0);\n\n");

}

/*
 *  Sds__outStructArrayNew
 *
 *  Output SdsNew call for a structure array.
 */
static void Sds__outStructArrayNew(
    StructDefType *s, 
    LexStringType *name, 
    int *level, 
    DimsType *dims,
    StatusType *status)
{
    if (*status != STATUS__OK) return;
/*
 *  Check the string length;
 */
    if ((strlen(name->str)+1) > SDS_C_NAMELEN)
    {
        if (SdsOutputMessages)
        {
            fprintf(stderr,"%s:%d:Name \"%s\" is too long for an SDS name, cannot be more then %d characters\n",name->filename,name->line_no,name->str,
                    SDS_C_NAMELEN);
            if (!SdsParsingError)
                SdsParsingError = SDS__TOOLONG;
        }
    }
    else
    {
/*
 *	Structured array.  We create both the structure itself and
 *	one at the next highest level so that we can create on copy
 *	of the contents of the array and then insert it thoughout.
 *
 *	Note that for anonymous level 0 structures, we replace the
 *	anonymous structure name with the item name.
 */
        Sds__outArrayDims(*level,dims,status);
        indent(*level);
        if (*level == 0)
        {
            if (SdsGenerateTcl)
            {
                printf("set tid0 [SdsNew %s  -dims $dims]\n",
                       ((strncmp(s->name.str,"_anon",5) == 0) ? 
                        name->str : s->name.str));
            }
            else
            {
                printf("SdsNew(0,\"%s\", 0 , NULL, SDS_STRUCT, %d, dims,\
&tid0, status);\n",
                       ((strncmp(s->name.str,"_anon",5) == 0) ? 
                        name->str : s->name.str),
                       dims->ndims);
            }
        }
        else
        {
            if (SdsGenerateTcl)
            {
                printf(
                    "set tid%d [SdsNew %s -parent #tid%d  -dims $dims]\n",
                    *level,name->str,((*level)-1));
            }
            else
            {
                printf("SdsNew(tid%d,\"%s\", 0 , NULL, SDS_STRUCT, %d, dims,\
 &tid%d, status);\n",
                       ((*level)-1),name->str,dims->ndims,*level);
            }
 
        }
        ++(*level);
        indent(*level);
        if (SdsGenerateTcl)
        {
            printf(
                "# Creating one invocation of the structure array member type\n");
            indent(*level);
            printf("set tid%d [SdsNew %s]\n",*level,s->name.str);
        }
        else
        {
            printf(
                "/*Creating one invocation of the structure array member type*/\n");
            indent(*level);
            printf("SdsNew(0,\"%s\", 0 , NULL, SDS_STRUCT, 0,NULL, \
&tid%d, status);\n",s->name.str,*level);
        }
    }
}

/*
 *   Sds__outStructNew
 *
 *   Output a SdsNew call for a non-array structure
 */
static void Sds__outStructNew(
    StructDefType *s, 
    LexStringType *name, 
    int level, 
    StatusType *status)
{
    if (*status != STATUS__OK) return;


/*
 *  Check the string length;
 */
    if ((strlen(name->str)+1) > SDS_C_NAMELEN)
    {
        if (SdsOutputMessages)
        {
            fprintf(stderr,"%s:%d:Name \"%s\" is too long for an SDS name, cannot be more then %d characters\n",name->filename,name->line_no,name->str,
                    SDS_C_NAMELEN);
            if (!SdsParsingError)
                SdsParsingError = SDS__TOOLONG;
        }
        return;
    }

/*
 *  A level 0 struct has a 0 topid.
 *
 *  Note that for anonymous level 0 structures, we replace the
 *  anonymous structure name with the item name.
 */
    indent(level);
    if (level == 0)
    {
        if (SdsGenerateTcl)
        {
            printf("set tid0 [SdsNew %s]\n",
                   ((strncmp(s->name.str,"_anon",5) == 0) ? 
                    name->str : s->name.str));
        }
        else
        {
            printf("SdsNew(0,\"%s\", 0 , NULL, SDS_STRUCT, 0, NULL,\
 &tid0, status);\n",
                   ((strncmp(s->name.str,"_anon",5) == 0) ? 
                    name->str : s->name.str));
        }
    }
    else
    {
        if (SdsGenerateTcl)
        {
            printf("set tid%d [SdsNew %s -parent $tid%d]\n",
                   level,name->str,(level-1));

        }
        else
        {
            printf("SdsNew(tid%d,\"%s\", 0 , NULL, SDS_STRUCT, 0, \
NULL, &tid%d, status);\n",(level-1),name->str,level);
        }
    }
 
}

/*
 *  Sds__outStructArrayFill
 *
 *  Fill a structure array.
 */
static void Sds__outStructArrayFill(
    StructDefType *s SDSUNUSED, 
    LexStringType *name SDSUNUSED, 
    int *level, 
    StatusType *status)
{


    if (*status != STATUS__OK) return;
    --(*level);
    indent(*level);
    if (SdsGenerateTcl)
    {
        printf("# Now fill the structure array with elements\n");
        indent(*level);
        printf("SdsFillArray $tid%d  $tid%d\n",*level,(*level)+1);
        indent(*level);
        printf("SdsDelete $tid%d\n",(*level+1));
        indent(*level);
        printf("SdsFreeId $tid%d\n",(*level+1));
    }
    else
    {
        printf("/* Now fill the structure array with elements */\n");
        indent(*level);
        printf("SdsFillArray(tid%d, tid%d, status);\n",
               *level,(*level)+1);
        indent(*level);
        printf("SdsDelete(tid%d,status);\n",(*level)+1);
        indent(*level);
        printf("SdsFreeId(tid%d,status);\n",(*level)+1);
    }

}
/*
 
 * Sds__outSdsStruct - Output code to create an sds structure.
 */
static void Sds__outSdsStruct(StructDefType *s, LexStringType *name, 
                              int level, DimsType *dims,
                              StatusType *status)
{
    StructElementType *list;
    if (*status != STATUS__OK) return;
/*
 *   First check for a valid structure definition
 */
    if (s == NIL)
    {
        
	fflush(stdout);	/* To align stdout with stderr	*/
	fprintf(stderr,"%s:%d:Invalid structure definition - \"%s\"\n",
                                       name->filename,name->line_no,name->str);
	*status = SDS__INVSTRUCTDEF;
    }
    else
    {
/* 
 *      If level 0 and we have a function name, output it and comment.
 */
	if ((level == 0)&&(SdsFunctionName))
            Sds__outLevelZeroFunc(s,status);

/*
 *	The level number allows us to define structures within structures
 *	by recursively calling this routine.  We create a new block for
 *	each structure so that we can declare new id's as needed.
 *
 *	Each block is indented 4 spaces for each level.
 */	
        if (!SdsGenerateTcl)
        {
            indent((level-1));  
            printf("{\n");
        }
	if ((level == 0)&&(!SdsFunctionName))
            Sds__outLevelZeroNoFunc(s,status);
	else	/* Otherwise, comment about the structure source line */
            Sds__outLevelNotZeroHead(dims->ndims,s,name,level,status);

        if (!SdsGenerateTcl)
            Sds__outCBlockVars(level,dims->ndims,status);
	  

/*
 *	For level 0, output dimension array if necessary, the scratch id.
 *	and the status check.
 */
	if ((level == 0)&&(!SdsGenerateTcl))
            Sds__outLevelZeroHead(status);

	if (dims->ndims)
            /* Output structured array SdsNew call */
            Sds__outStructArrayNew(s,name,&level,dims,status);

	else
            /* Output non-array structure SdsNew call */
            Sds__outStructNew(s,name,level,status);
	
/*
 *	Output the elements of the structure.
 */
        list = Sds__linkedList(s->first);
        Sds__outStructElem(list,level,status);
        Sds__linkedListRepair(s->first,list);
/*
 *      If a structure array, output code to fill it up.
 */        
	if (dims->ndims)
            Sds__outStructArrayFill(s,name,&level,status);
	
/*
 *      Free the structure id.
 */
	if (level != 0)
	{
	    indent(level);
            if (SdsGenerateTcl)
                printf("SdsFreeId $tid%d;\n",level);
            else
                printf("SdsFreeId(tid%d,status);\n",level);
	}
	indent(level);
/*
 *      Block postscript.
 */
        if (SdsGenerateTcl)
            printf("# End of structure definition at level %d\n\n",level);
        else
            printf("/* End of structure definition at level %d */\n\n",level);
	if (level == 0)
	{
	    indent(0);
            if (SdsGenerateTcl)                
                printf("return $tid0\n}\n");
            else
                printf("return(tid0);\n}\n");
	}
	else
	{
            if (!SdsGenerateTcl)                
            {
                indent((level-1));
                printf ("}\n");
            }
	}
    }
}

/*
 
 * Sds__outStructElem - Recursively output structure definition
 */

static void Sds__outStructElem(StructElementType *elem, int level, 
				StatusType *status)
{
    if (*status != STATUS__OK) return;

/*
 *  If the element type is unknown, we have an error
 */
    if ((elem->type < SDS_PRIM_MIN)&&(elem->type != SDS_STRUCT))
    {
	*status = SDS__INVTYPE;
	if (SdsOutputMessages)
	    fprintf(stderr,"%s:%d:Invalid SDS type %d\n",elem->name.filename,
						         elem->name.line_no,
						         elem->type);
    }
/*
 *  If the element is a structure, output it by recursively calling OutSdsStruct
 */     
    else if (elem->type == SDS_STRUCT)
	Sds__outSdsStruct(elem->structPtr,&elem->name, level+1, 
                          &elem->dims, status);
/*
 *  If this is a primitive element, output it
 */
    else if (elem->type <= SDS_PRIM_MAX)
    {
/*
 *       Check the string length;
 */
        if ((strlen(elem->name.str)+1) > SDS_C_NAMELEN)
        {
            if (SdsOutputMessages)
            {
                fprintf(stderr,"%s:%d:Name \"%s\" is too long for an SDS name, cannot be more then %d characters\n",elem->name.filename,elem->name.line_no,
                        elem->name.str,SDS_C_NAMELEN);
                if (!SdsParsingError)
                    SdsParsingError = SDS__TOOLONG;
            }
            return;
        }

/*
 *	If the number of dimensions are greater then 0, then we must
 *	setup the dimensions array.
 */
	if (elem->dims.ndims != 0)
	{
	    Sds__outArrayDims(level,&elem->dims,status);
    	    indent(level);
            if (SdsGenerateTcl)
            {
                printf(
                    "set id [SdsNew %s -parent $tid%d -dims $dims -type %s]\n",
                    elem->name.str,level,TclSdsCodes[elem->type]);
            }
            else
            {
                printf("SdsNew(tid%d, \"%s\", 0, NULL, %s, %d , dims, &id, status);\n",
    		level,elem->name.str,SdsCodes[elem->type],
    				elem->dims.ndims);
            }
	}
/*
 *	Otherwise, just output a simple SdsNew call
 */
	else
	{
	    indent(level);
            if (SdsGenerateTcl)
            {
                printf("set id [SdsNew %s -parent $tid%d -type %s]\n",
			elem->name.str,level,TclSdsCodes[elem->type]);
            }
            else
            {
                printf("SdsNew(tid%d, \"%s\", 0, NULL, %s, 0 , NULL, \
 &id, status);\n",
			level,elem->name.str,SdsCodes[elem->type]);
            }
	}
	indent(level);
        if (SdsGenerateTcl)
            printf("SdsFreeId $id\n");
        else
            printf("SdsFreeId(id,status);\n");
    }
    else
    {
	*status = SDS__INVTYPE;
	if (SdsOutputMessages)
	    fprintf(stderr,"%s:%d:Invalid SDS type %d\n",elem->name.filename,
						         elem->name.line_no,
						         elem->type);
    }
/*
 *   Output the next element in the list
 */
    if (elem->next != NIL)
	Sds__outStructElem(elem->next,level,status);


}
/*
 *   Output the array dimensions setup for an array at the specified level.
 */
static void Sds__outArrayDims(int level,
		      DimsType *dims,
		      StatusType *status)
{
    register unsigned i;

    if (*status != STATUS__OK) return;

    if (SdsGenerateTcl)
    {
        indent(level);
        printf("set dims {");
        for (i = 0 ; i < (unsigned)dims->ndims ; ++i)
        {
            printf("%ld ",dims->dims[i]);
        }
        printf("}\n");
    }
    else
    {
        for (i = 0 ; i < (unsigned)dims->ndims ; ++i)
        {
            indent(level);
            printf("dims[%d] = %ld;\n",i,dims->dims[i]);
        }
    }
}

#endif /* VxWorks */
/*
  
 *  Sds__createSdsStruct - Create the SDS structure in the current process.
 */
static SdsIdType Sds__createSdsStruct(StructDefType *s, LexStringType *name, 
                                      SdsIdType tid, DimsType *dims, 
                                      StatusType *status)
{
    SdsIdType id=0;		/* If of the structure created here	*/
    if (*status != STATUS__OK) return(0);
/*
 *   First check for a valid structure definition
 */
    if (s == NIL)
    {
	*status = SDS__INVSTRUCTDEF;
	return 0;
    }
    if (dims->ndims)
    {
/*
 *	    Structure is multi dimensional.  We create the top level and
 *	    then one version of the contents.  We then fill the top level
 *	    with copies of the contents.
 */
        SdsIdType sid;
        StructElementType *list;
        /* Create structure array */
        if (tid == 0)
            SdsNew(tid, s->name.str, 0, NULL, SDS_STRUCT, dims->ndims, 
                   dims->dims, &id, status);
        else
            SdsNew(tid,name->str, 0, NULL, SDS_STRUCT, dims->ndims, 
                   dims->dims, &id, status);

/*
 *	   Create one version of the structure to be inserted and put
 *	   the contents into it.
 */
        SdsNew(0,s->name.str, 0, NULL, SDS_STRUCT, 0, NULL, &sid,status);
        list = Sds__linkedList(s->first);
        Sds__createStructElem(list,id,status);
        Sds__linkedListRepair(s->first,list);
/*
 *	   Now fill the structured array with this and delete the temp
 */	   
        SdsFillArray(id,sid,status);
        SdsDelete(sid,status);
        SdsFreeId(sid,status);
    }
    else
    {
        StructElementType *list;
        if (tid == 0)
            SdsNew(tid, s->name.str, 0, NULL, SDS_STRUCT, 0, NULL, &id, status);
        else
            SdsNew(tid,name->str, 0, NULL, SDS_STRUCT, 0, NULL, &id, status);
/*
 *	    Create the elements of the structure
 */
        list = Sds__linkedList(s->first);
        Sds__createStructElem(list,id,status);
        Sds__linkedListRepair(s->first,list);
    }       
    return(id);
}

/*
 
 * Sds__createStructElem - Recursively output structure elements.
 */

static void Sds__createStructElem(StructElementType *elem, SdsIdType tid, 
				  StatusType *status)
{
    SdsIdType id;
    if (*status != STATUS__OK) return;

/*
 *  If the element type is unknown, we have an error
 */
    if ((elem->type < SDS_PRIM_MIN)&&(elem->type != SDS_STRUCT))
    {
	*status = SDS__INVTYPE;
	if (SdsOutputMessages)
	    fprintf(stderr,"%s:%d:Invalid SDS type %d\n",elem->name.filename,
						         elem->name.line_no,
						         elem->type);
    }
/*
 *  If the element is a structure, create it by recursively calling OutSdsStruct
 *  and then free the id.
 */     
    else if (elem->type == SDS_STRUCT)
	SdsFreeId(Sds__createSdsStruct(elem->structPtr,
                                       &elem->name, tid, &elem->dims,status),
                  status);
/*
 *  If this is a primitive element, create it and free the id.
 */
    else if (elem->type <= SDS_PRIM_MAX)
    {
	SdsNew(tid,elem->name.str, 0, NULL, elem->type, 
					    elem->dims.ndims,
					    elem->dims.dims, 
					    &id, status);
	SdsFreeId(id,status);
    }
    else
    {
	*status = SDS__INVTYPE;
	if (SdsOutputMessages)
	    fprintf(stderr,"%s:%d:Invalid SDS type %d\n",elem->name.filename,
						         elem->name.line_no,
						         elem->type);
    }
/*
 *   Output the next element in the list
 */
    if (elem->next != NIL)
	Sds__createStructElem(elem->next,tid,status);


}

/*
 *+	S d s C o m p i l e r
 *
 * Function Name:
 *	SdsCompiler
 *
 * Function:
 *      Compile a C structure definition to create an SDS structure.
 *
 * Description:
 *      When given a string containing a valid C structure definition,
 *      this routine will create a Sds version of the structure and return
 *      its id.  This routine is just a run time hook into the "sdsc" program.
 *
 *	Note, that this routines generated by lex and yacc and hence uses
 *	any external names in the code generated by those programs.
 *
 * 
 * Declaration: 
 *	void SdsCompiler(char *string, int messages, int intas32bit, SdsIdType *id, StatusType *status);
 *
 * Parameters: (">" input, "!" modified, "W" workspace, "<" output)
 *    (>) string	(char *)  A string containing a C structure declaration
 *    (>) messages	(int)	If true, then in addition to setting status,
 *			messages will be written to the stderr when 
 *			parsing error occur.  This may help in debugging as
 *			only one status value can be returned.  There are
 *			two possible values
 *			SDS_COMP_MESS_ERROR => Output messages when status
 *				would be set.
 *			SDS_COMP_MESS_WARN => Inaddition to error messages,
 *				output warnings.
 *    (>) intas32bit	(int) Set true to treat ambiguous int declarations as
 *			32 bit ints.  Clear to treat them as short ints.
 *    (<) id		(SdsIdType *) The id of the created structure.
 *    (!) status	(StatusType *) Modified status. 
 *			Possible failure codes are:
 *			SDS__INVPRIMTYPE => Invalid primitive type code.
 *			SDS__INVSTRUCTDEF => Invalid structure definition.
 *			SDS__SYNTAX => Parser syntax error.
 *			SDS__INVSTRUCTURE => Invalid structure.
 *			SDS__INVTYPETYPE => Invalid typedef type.
 *			SDS__STRUCTMULTDEF => Multiply defined structure.
 *			SDS__INVINT => Invalid integer.
 *			SDS__INVTYPE => Invalid type.
 *			SDS__INVINPUT => Invalid input.
 *			SDS__STRUCTARRAY => Array of structures.
 *			SDS__MAXDIMS => Exceeded maximum number of dimensions.
 *			SDS__NOINPUT => String was empty.
 *                      SDS__TOOLONG => A string which is used as an SDS
 *                                      item name is too long.
 *
 * Prior requirements:
 *	None
 *
 * Support: Tony Farrell, AAO
 *
 * Version date: 17-Jun-92
 *-
 *	
 */

void SdsCompiler(SDSCONST char *string, int messages, int intas32bit, SdsIdType *id, StatusType *status)
{
   if (*status != STATUS__OK) return;
   SdsInputFile = 0;			/* No input file used	        */
   SdsInputString = string;		/* Used the supplied string instead */
   SdsDims = 0;				/* Clear the number of dimensions */
   SdsIntAs32Bit = intas32bit;		/* How to tread ambiguous ints	*/
   Structures = NIL;			/* Clear the lists	        */
   Types = NIL;			
   strcpy(SdsFileName,"Line");		/* File name in output messages */
   SdsOutputMessages = messages;	/* What to do with messages	*/
   SdsOutputCode = 0;			/* Do not output code		*/
   SdsParsingError = STATUS__OK;		/* Clear parsing error code	*/
   SdsYyparse();	       		/* Do the parse			*/
   if (SdsParsingError)
   {
	*status = SdsParsingError;
	if (SdsOutputMessages)
	    fprintf(stderr,"Parsing failed, first error was %d\n",SdsParsingError);
   }
   *id = SdsId;
}
