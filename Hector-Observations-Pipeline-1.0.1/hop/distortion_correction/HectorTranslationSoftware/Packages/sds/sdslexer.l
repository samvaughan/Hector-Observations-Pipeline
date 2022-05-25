%{
/*
 *	Self-defining Date System
 *
 *	C structure compiler, lexical analyzer generator
 *
 *	Tony Farrell, 17-Jul-1992.
 * 
 *
 *	This module should be run though the unix utility lex and the result
 *	renamed as sdslexer.c.  This file can then be compiled in the normal 
 *	way.  Before sdslexer.c can be compiled under VMS, a couple of
 *	incomplatible constructs must be removed.  See the makefile for
 *	details.
 *
 *  @(#) $Id: ACMM:sds/sdslexer.l,v 3.94 09-Dec-2020 17:15:55+11 ks $
 *
 * History:
 *      17-Jul-1992 - TJF - Original version.
 *      01-Aug-1996 - TJF - Add INT32/UINT32/INT64/UINT64
 *      07-Jan-2003 - TJF - Include string.h.
 *      05-Apr-2004 - TJF - Tidy up flex handling.
 */
#include "status.h"
#include "sdscompiler.h"
#include "sdsparser.h"
#include <string.h>
int yywrap(void) { return(1); }

/*
 * Internal function prototypes
 */
static void Sds__flushComment();
static int  Sds__getc(FILE *);
static void Sds__saveFilename();
 
/*
 *  We have our own version of the Lex ECHO macro.  It only does anything
 *	when DEBUG is defined.
 */
#undef ECHO
#ifdef DEBUG
#   define ECHO printf ("%s on line %d\n",yytext,yylineno);
#else
#   define ECHO
#endif
/* 
 * Our own input routine, so the we can take input from a string as well as
 * stdin.  Sds__getc is the acutal input routine.
 *
 *  If "input" is a define macro - then we are running in the original "lex"
 *  (e.g. Solaris) and we have to refine "input" appropiately.
 *
 * If YY_FLEX_MAJOR_VERSION is defined and is at least 2, then we are
 * running in a recent version of "flex" - say the lex command
 * under Linux or MacOsX.  In this case we want to define YY_INPUT - which
 * takes three arguments - the buffer, the result variable and the size of
 * the buffer.
 *
 * Otherwise we presume an older version of FLEX (no current example)
 * where we have to define YYINPUT. 
 *
 * In both the "flex" cases, the "input" function is still defined and
 *  therefore can be called from things such as Sds__flushComment().
 */
#ifdef input
#    undef input
#    define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):Sds__getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#elif defined(YY_FLEX_MAJOR_VERSION) && (YY_FLEX_MAJOR_VERSION >= 2)
#    define YY_NO_UNPUT  /* Gets rid of warning message when compiling */
     int yylineno;       /* Keeps track of the line number */
#    define YY_INPUT(buf,result,max_size) \
     { \
         int c = Sds__getc(yyin); \
         if (c == 10) ++yylineno; \
         result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
     }

#else
     int yylineno;
#    define YYINPUT() (((yytchar=yysptr>yysbuf?U(*--yysptr):Sds__getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)

#endif
/*
 *  The COMRET implements a common return sequence of setting yylval to the line
 *   number and returning a token name.
 */
#define COMRET(A) { ECHO yylval.lnum = yylineno ; return(A) ; }
/*
 *  Now Setup for Lex itself.
 */
%}
ws	[ \t\n\f]+
name	[A-Za-z_][\$A-Za-z0-9_]*
hexint  0[xX][0-9A-Fa-f]+
integer [0-9]+
file    #[ ]+[0-9]+[ ]+\"[^\"\n]+\"


%%
char			{ COMRET(tCHAR) }
int			{ COMRET(tINT) }
INT64			{ COMRET(tINT64) }
INT32			{ COMRET(tINT32) }
UINT32			{ COMRET(tUINT32) }
UINT64			{ COMRET(tUINT64) }
float			{ COMRET(tFLOAT) }
double			{ COMRET(tDOUBLE) }
unsigned		{ COMRET(tUNSIGNED) }
signed			{ COMRET(tSIGNED) }
short			{ COMRET(tSHORT) }
long			{ COMRET(tLONG) }
enum			{ COMRET(tENUM) }
struct			{ COMRET(tSTRUCT) }
typedef			{ COMRET(tTYPEDEF) }
extern			{ COMRET(tEXTERN) }
static			{ COMRET(tSTATIC) }
const			{ COMRET(tCONST) }
volatile		{ COMRET(tVOLATILE) }
register		{ COMRET(tREGISTER) }
auto			{ COMRET(tAUTO) }
{hexint}		{ /* Hexadecimal Integers  */
			  ECHO
			  Sds__makeString(yytext,&yylval.integer);
			  return(tINTEGER); }
{integer}		{ /* Octal or decimal Integers	  */
			  ECHO
			  Sds__makeString(yytext,&yylval.integer);
			  return(tINTEGER); }
{name}			{ /* standard names */
			  ECHO
			  Sds__makeString(yytext,&yylval.integer);
			  return(tNAME); }
\[			{ return('[');	}
\]			{ return(']');  }
;			{ return(';');  }
\{			{ return('{');	}
\}			{ return('}');	}
\,			{ return(',');	}
=			{ return('=');  }
\/\*			{ ECHO
			  Sds__flushComment();	}
{ws}			{	/* white space	*/;	}
{file}		  	{   /* This should be able to be used to */   
			    /* return the file name and line number */
			    /* as output for a C preprocessor, but  */
			    /* is does not work, so the following   */
			    /* does the job			    */
				Sds__saveFilename();		}
#.*$		  	{  	Sds__saveFilename();		}
.			{ /* The rest, invalid input */
			    if (SdsOutputMessages)
				fprintf(stderr,"%s:%d:Invalid input - \"%s\"\n",
						SdsFileName,yylineno,yytext);
			  if (!SdsParsingError) SdsParsingError = SDS__INVINPUT; }
%%
static void Sds__flushComment()
{
/*
 *  It is easier to handle comments at this level.  When lex indicates the
 *  comment initiator has been input, it calls this routine and we read
 *  until we get the comment terminator.   It also allows us to warn about
 *  nested comments
 */

    int c;
    int start_line = yylineno;		/* Save the starting line number */
#   ifdef DEBUG
	printf("Flushing comment\n");
#   endif
	
    do 
    {
/*
 *	Read a character.  If we get a comment terminator, break.  If we get
 *		initiator, output a message.  A 0 indicates EOF or Error
 */
	c = input();
	if ((c == '*')&&((c = input()) == '/'))
		break;
        if ((c == '/')&&((c = input()) == '*')&&(SdsOutputMessages))
	    fprintf(stderr,"%s:%d:Nested comment initiator ignored\n",
                           SdsFileName,yylineno);
	
	if ((c == 0)&&(SdsOutputMessages))
	    fprintf(stderr,"%s:%d:End of file occurred in the comment which started\
 on line %d\n", SdsFileName,yylineno,start_line);
  
    }    while (c != 0);
}
static int Sds__getc(FILE * in)
/*
 *  This routine allows us to read from alterative sources, in particular,
 *  a string.  If the flag SdsInputFile is true, we read from the string 
 *  pointed to by SdsInputString, which points to the next character.
 */
{
    static int first = 1;
    if (SdsInputFile)
    {
/*
 *	The strange initialisation of yyin and yyout done by default in
 *	lexers generated by lex is not suitable for VMS so a sed line
 *	has got rid of it and we must do it here.
 */
	if (first)
	{
	    yyin = stdin;
	    yyout = stdout;
	    in = stdin;
	    first = 0;
	}
	return(getc(in));
    }
    else
    {
	if (*SdsInputString == '\0')
	    return(EOF);
        else
	    return(*SdsInputString++);
    }
}
static void Sds__saveFilename()
{
/*
 *  This routine is called when we get a line of the form 
 *      # integer "name"
 *
 *  This type of line is output by the C preprocessor to indicate
 *  the following code is from the file specified by the name, starting
 *  at the line number specified by the integer.  We have this information 
 *  in order to make out error messages sensible.
 *
 */
    int i; 
    sscanf(yytext,"# %d \"%s\"\n",&yylineno,SdsFileName);
    --yylineno;
    i = strlen(SdsFileName);
    --i;
/*
 *  The above sscanf my pickup the trailing quite, so remove it.
 */
    if (SdsFileName[i] == '\"')
	SdsFileName[i] = '\0';
/*
 *  If the length is zero, we are reading the stdin, so set the file name
 */
    if (i == 0) 
        strcpy(SdsFileName,"<stdin>");

}
