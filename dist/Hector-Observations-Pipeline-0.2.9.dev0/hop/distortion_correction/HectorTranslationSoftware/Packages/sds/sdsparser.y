%{
/*	
 *	Self-defining Date System
 *
 *	C structure compiler, parser generator generator.
 *
 *	Tony Farrell, 17-Jul-1992.
 * 
 *	This module contains a parser which will parse sds definition files.  
 *      After being run through yacc, a C module will be created who's entry 
 *      point is yyparse.  This C module should be renamed as sdsparser.c.  In
 *	addiition, when run with the -d option, yacc generates an include
 *	file which should be renamed as sdsparser.h, for used by the lexer.
 *
 *	The module lexer.c, created by running lexer.l though lex is required.
 *
 *  @(#) $Id: ACMM:sds/sdsparser.y,v 3.94 09-Dec-2020 17:15:54+11 ks $ 
 *
 * History:
 *     17-Jul-1992 - TJF - Original version.
 *     08-Jul-1993 - TJF - Change from VXWORKS macro to VxWorks
 *     01-Aug-1996 - TJF - Support arrays of structures. Support 64 bit ints.
 *     05-Apr-2004 - TJF - Merge in changes from Nick Rees @ JACH, which
 *                         help support WIN32 compilation.
 *                         Prototype yylex for BISON generated parsers.
 *     04-Jun-2012 - TJF - YYBYACC not used in later bison, also check for YYBISON
 *     04-Apr-2018 - TJF - By default, now prohibit use of "int", "long int" and
 *                           releated types to avoid ambigous definitions.  The
 *                           "-c" option allows this behaviour to revert to the old
 *                           approach.
 */
#ifdef VxWorks
#include <stdioLib.h>
#else
#include <stdio.h>
#endif
#include "sdscompiler.h"

/*
 * YYBYACC or YYBISON will (I believe) be defined if we are being built 
 * by bison rather then yacc.  
 */
#if defined(YYBYACC)||defined(YYBISON)
    int yylex(void);  /* This is not prototyped with bison() */
#endif

/*
 *  When we have an array, we build its dimensions in these variables, 
 *	which are passed to MakePrimitiveArray etc.  This limits the
 *	number of dimensions to MAXDIMS, but is much easier then building
 *	a list containing the dimensions.  Note that the dimensions are
 *	stored here as string representaions of integers, which will be
 *	converted to integers later.
 */

static int ndims = 0;
static LexStringType dims[MAXDIMS];
/*
 * Sds__yyerror is the  local error output routines which adds the current 
 * lex line number and file name to the message and sets the SdsParsingError
 * variable.
 *
 * yyerror must go though Sds__yyerror to add the null 
 */	
#define yyerror(A) Sds__yyerror((A), (char *)0)
void Sds__yyerror(char *s, char *t)
{
    extern int yylineno;
    if (SdsOutputMessages)
    {
        fflush(stdout);	/* To align stdout with stderr */
        fprintf(stderr,"%s:%d:Parsing failure - %s",
                                                     SdsFileName,yylineno,s);
        if (t)
            fprintf(stderr," %s",t);
        fprintf(stderr,"\n");
    }
    if (!SdsParsingError)
        SdsParsingError = SDS__SYNTAX;
}
/*
 * Handle the problem of int declarations being ambiguous (32bit vs 16 bit)
 *   If SdsIntCompatibity is false, then this is a syntax
 *   error.
 *
 *  Otherwise, If the flag  Sds__intAsLong is true, return the 
 * long version, else return the short  version.
 */
static int Sds__handleInts(char *type,int  isunsigned)
{
    extern int yylineno;

    if (!SdsIntCompatibility)
    {
        fflush(stdout);  /* To align stdout with stderr */
	fprintf(stderr,
                "%s:%d:\"%s\" is an ambiguous declaration (its size varies between machines). Suggest replacing with %s\n",
		SdsFileName,yylineno,type, 
                (isunsigned ? "UINT32": "INT32"));
        fprintf(stderr,
                "     Specify the \"-c\" argument to revert to old approach - if you really must!\n");
        SdsParsingError = SDS__AMBIGIOUS_TYPE;
        return 0;
        
    }

    if (SdsOutputMessages&SDS_COMP_MESS_WARN)
    {
	fflush(stdout);	/* To align stdout with stderr */
	fprintf(stderr,"%s:%d:\"%s\" is an ambiguous declaration (its size varies between machines). I am assuming it is of sds type \"%s int\"\n",
		SdsFileName,yylineno,type,(SdsIntAs32Bit? "32 bit" : "short")); 
    }
    if (isunsigned)
	return(SdsIntAs32Bit ? SDS_UINT : SDS_USHORT);
    else
	return(SdsIntAs32Bit ? SDS_INT : SDS_SHORT);
}




/*
 * Handle the problem of long declarations being ambiguous (32bit vs 64 bit)
 *   If SdsIntCompatibity is false, then this is a syntax
 *   error.
 *
 *  Otherwise, If the flag  Sds__intAsLong is true, return the 
 * long version, else return the short  version.
 */
static int Sds__handleLong(char *type,int  isunsigned)
{
    extern int yylineno;

    if (!SdsIntCompatibility)
    {
        fflush(stdout);  /* To align stdout with stderr */
	fprintf(stderr,
                "%s:%d:\"%s\" is an ambiguous declaration (its size varies between machines). Suggest replacing with %s\n",
		SdsFileName,yylineno,type, 
                (isunsigned ? "UINT32/UINT64": "INT32/INT64"));
        fprintf(stderr,
                "     Specify the \"-c\" argument to revert to old approach - if you really must!\n");
        SdsParsingError = SDS__AMBIGIOUS_TYPE;
        return 0;
        
    }

    if (SdsOutputMessages&SDS_COMP_MESS_WARN)
    {
	fflush(stdout);	/* To align stdout with stderr */
	fprintf(stderr,"%s:%d:\"%s\" is an ambiguous declaration (its size varies between machines). I am assuming it is of sds type \"%s\"\n",
		SdsFileName,yylineno,type,
                (isunsigned ? "UINT32": "INT32")); 
        fprintf(stderr,"   But this WILL BE WRONG on 64 bit machines\n");
    }
    if (isunsigned)
	return(SDS_UINT);
    else
	return(SDS_INT);
}




%}

/*
 * The union represents each of the types that may be returned by yylex
 *  or action routines as the value of the token
 */
%union {
	int  lnum;		/* The line number		*/
	LexStringType name;	/* A name (string and line num)	*/
	LexStringType integer;	/* ASCII representation of an integer */
	StructElementType *structelem;	/* A struct element	*/	
	int prim;		/* A primitive type 		*/
	StructDefType *structure;/* Structure details		*/
	char *s;		/* for strings			*/
}
/*
 *  define each of the terminal tokens.
 */
%token <lnum> tCHAR
%token <lnum> tUNSIGNED
%token <lnum> tSHORT
%token <lnum> tLONG
%token <lnum> tSIGNED
%token <lnum> tINT
%token <lnum> tFLOAT
%token <lnum> tDOUBLE
%token <lnum> tENUM

%token <lnum> tINT32
%token <lnum> tUINT32
%token <lnum> tINT64
%token <lnum> tUINT64

%token <lnum>  tSTRUCT
%token <lnum>  tTYPEDEF

%token <lnum> tREGISTER
%token <lnum> tCONST
%token <lnum> tVOLATILE
%token <lnum> tEXTERN
%token <lnum> tSTATIC
%token <lnum> tAUTO

%token <name> tNAME
%token <integer> tINTEGER

/*
 * Now the non terminal's type definitions
 */
%type <structelem> structItemList structItem
%type <prim> primitive
%type <structure> structure

/*
 *  Now the language definition.
 *	Note that the opening brace "{" in each action statement should
 *	be immediately followed by at least one space.  This allows 
 *	and ed editor session using "sdsparser.ed", to generate the
 *	grammer from this file.
 */
%%
module  : declaration 				 
	| definitionList declaration		 	
	| definitionList		{ Sds__generateStructNamed();  }
	| /* Empty input file */	{ SdsParsingError = SDS__NOINPUT; }
	;

definitionList : definitionList definition
	| definition
	;

definition : typedefinition semicolon
	|   error  		        {  yyerror(
         "Invalid token when structure definition or typedef required "); }
	;

typedefinition :    structure	{ }			
	| typedef { }
	;

semicolon : ';'
	|    error		{ yyerror("Missing semicolon"); }
	;

storage : storage storageitem
	| storageitem
	;

storageitem : tREGISTER { }
	|     tCONST { }
	|     tVOLATILE { }
	|     tEXTERN { }
	|     tSTATIC { }
	|     tAUTO { }
	;

typedef : tTYPEDEF primitive tNAME 	{ Sds__primitiveType ($2, &$3, 0,
          (LexStringType *)0); }
	| tTYPEDEF structure tNAME      { Sds__structureType ($2, &$3, 0,
 						         (LexStringType *)0) ; }
	| tTYPEDEF tNAME tNAME	     	{ Sds__typeType      (&$2, &$3, 0,
 						         (LexStringType *)0) ; }
	| tTYPEDEF primitive tNAME arrayspec { Sds__primitiveType 
						       ($2, &$3, ndims, dims); }
	| tTYPEDEF structure tNAME arrayspec { Sds__structureType ($2, &$3,
							ndims, dims); }
	| tTYPEDEF tNAME tNAME arrayspec { Sds__typeType (&$2, &$3, ndims,dims);}
	;

declaration : structure tNAME ';'	{ Sds__generateStruct($1, &$2,0,0) ; }
        | storage structure tNAME ';'	{ Sds__generateStruct($2, &$3,0,0) ; }
	| tNAME tNAME ';'       	{ Sds__generateStruct(
					Sds__lookupSType(&$1), &$2,0,0) ;}
	| storage tNAME tNAME ';'       { Sds__generateStruct(
			 		Sds__lookupSType(&$2), &$3,0,0) ;}
						
        | storage structure tNAME arrayspec';'	
        			{ Sds__generateStruct($2, &$3,ndims,dims) ; }
	| tNAME tNAME arrayspec ';'      { Sds__generateStruct(
				Sds__lookupSType(&$1), &$2,ndims,dims) ;}
	| storage tNAME tNAME arrayspec';' { Sds__generateStruct(
				Sds__lookupSType(&$2), &$3,ndims,dims) ;}
	;


structure : tSTRUCT tNAME '{' structItemList '}' { $$=Sds__makeStruct(&$2,$4); }
	|   tSTRUCT '{' structItemList '}' 	 { $$=Sds__makeStruct(0,$3); }
	|   tSTRUCT tNAME   			 { $$=Sds__lookupStruct(&$2);}
	;

structItemList : structItemList structItem { $$ =Sds__joinElementLists($1,$2); }
	|	 structItem		   { $$ = $1; }
	;

structItem : primitive tNAME semicolon { $$ = 
	  Sds__primitiveElement($1, &$2,0,(LexStringType *)0);}
	|    structure tNAME semicolon { $$ = 
			Sds__structureElement($1, &$2, 0, (LexStringType *)0);}
	|    tNAME     tNAME semicolon { $$ = Sds__typedefElement(
          Sds__lookupType(&$1), &$2, 0, (LexStringType *)0) ;}
	|    primitive tNAME arrayspec semicolon { $$ = 
				   Sds__primitiveElement($1, &$2,ndims,dims);}
	|    structure tNAME arrayspec semicolon { $$ = 
				   Sds__structureElement($1, &$2,ndims,dims);}
	|    tNAME tNAME arrayspec semicolon { $$ = Sds__typedefElement(
                                   Sds__lookupType(&$1), &$2, ndims, dims) ;}
	;

arrayspec : arrayspec '[' tINTEGER ']'	{ dims[ndims++] = $3; }
	| '[' tINTEGER ']'		{ ndims = 0;  dims[ndims++] = $2; }
	;

primitive :  	tCHAR 			{ $$ = 	SDS_CHAR; }		
	|    	tUNSIGNED tCHAR		{ $$ =  SDS_UBYTE; }
	|   	tSIGNED tCHAR 		{ $$ =  SDS_BYTE; }
	|	tSHORT			{ $$ =  SDS_SHORT; }
	|	tUNSIGNED tSHORT	{ $$ =  SDS_USHORT; }
	|	tSIGNED tSHORT		{ $$ =  SDS_SHORT; }
	|	tSHORT tINT		{ $$ =  SDS_SHORT; }
	|	tUNSIGNED tSHORT tINT	{ $$ =  SDS_USHORT; }
	|	tSIGNED tSHORT tINT	{ $$ =  SDS_SHORT; }
        |	tINT			{ $$ =  Sds__handleInts("int",               0); }
        |	tSIGNED tINT		{ $$ =  Sds__handleInts("signed int",        0); }
        |	tUNSIGNED tINT	        { $$ =  Sds__handleInts("unsigned int",      1); }	
        |	tLONG 			{ $$ =  Sds__handleLong("long",              0); }
        |	tLONG tINT		{ $$ =  Sds__handleLong("long int",          0); }
        |	tSIGNED tLONG		{ $$ =  Sds__handleLong("signed long",       0);}
        |	tSIGNED tLONG tINT	{ $$ =  Sds__handleLong("signed long int",   0); }
        |	tUNSIGNED tLONG		{ $$ =  Sds__handleLong("unsigned long",     1); }
        |	tUNSIGNED tLONG tINT	{ $$ =  Sds__handleLong("unsigned long int", 1); }
	|	tFLOAT			{ $$ =  SDS_FLOAT; }
	|	tDOUBLE			{ $$ =  SDS_DOUBLE; }
	|	tINT32			{ $$ =  SDS_INT; }
	|	tUINT32			{ $$ =  SDS_UINT; }
	|	tINT64			{ $$ =  SDS_I64; }
	|	tUINT64			{ $$ =  SDS_UI64; }
        |       tENUM tNAME             { $$ =  SDS_INT; }		
	|       tENUM tNAME '{' enumlist '}' { $$ = SDS_INT; }
	|       tENUM '{' enumlist '}'  { $$ = SDS_INT; }	
	|	error			{ yyerror(
	"Invalid token when primitive type required"); }
	;

enumlist : enumlist ',' enumname 
	|  enumname
	;
	 
enumname : tNAME  { }
	|  tNAME '=' tINTEGER  { }
	;
