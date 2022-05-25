/*+		S D S C
 *
 * Function Name:
 *	sdsc
 *
 * Function:
 *      Compiles C structure definitions into SDS Calls. 
 *
 * Synopsis:
 *v
 *v    sdsc [ -nvlswT] [ -Dname ] [ -Dname=def ] [ -Uname ] 
 *v         \[ -ffunction ]  [ -ttype ] [ -Nname ]
 *v         \[ -Idirectory ] [ -Pcpp ] [ input-file ] [ output-file]
 *
 * Description:
 * 	A C structure can be put and retrived from a similar SDS
 *	structure using SdsPut and SdsGet.  This makes it desirable to be
 *	able to automatically generate SDS calls to produce an SDS
 *	structure equivalent to the C structure.  sdsc does this job.
 *
 *	The input to sdsc is frist run through a C preprocessor.  During
 *	this the macros 'SDS' and '___SDS___' will be defined (in addition)
 *	to any macros defined by default).   The result should be a series
 *	of C definitions followed by one and only one structure declaration.
 *
 *      sdsc optionally accepts two filenames as arguments.  input-file and
 *	output-file are, respectively, the input and output files.  They
 *	default to the standard input and standard output (except under VMS,
 *	when you must specifiy the input file (due to problems with the C
 *	run time library under the current version of VMS)).
 *
 * Options:
 *	[-n]	Do not run the input through the C preprocessor.  
 *	[-v]	Output the command used to run the  C preprocessor to stderr.
 *      [-l]	Treat ambiguous int declarations as 32bit ints.
 *      [-s]      Treat ambiguous int declarations as short ints.
 *      [-w]      Instead of outputing declarations, use the SdsWrite
 *		utility function to write the Sds structure to a file .  
 *		In this case, an output file must be supplied.
 *      [-W]    Output warning messages as well as error messages.
 *      [-T]    Output Tcl code instead of C code.  This requires the Sds
 *              commands in DRAMA's Dtcl package (DRAMA V1.2 and later only)
 *      [-c]    Accept "int", "long int" etc. in the input file.  From SDS 1.4.3.87,
 *                the default is to treat these as error since they are not
 *                well defined types and can give 32/64 bit compatiblity issues.
 *	[-Dname]  Define name as 1 (one).  This is the same as if a -Dname=1
 *		option appeared on the command line, or as if a
 *v
 *v                #define name 1
 *		appeared in the source file.
 *
 *      [-ffunction] Specify a function name.  When specified, instead of
 *             outputing just the body of the function, then entire
 *             function is output, this is the name for the function.
 *      [-ttype] Specify a type, a declaration of which is to be assumed
 *             at the end of reading the input file if the input fie
 *             does not declare a variable.  It should be a simple type
 *             or a typedef name.
 *      [-Nname] If -t is used, then this name is given to the variable
 *             created and hence to the top level structure.  If not supplied,
 *             then the variable type is used as the variable name.
 *	[-Dname=def] Define name as if by a #define directive.  
 *	[-Uname]	Remove any initial definition of name where name is a
 *		symbol predefined by the C preprocessor.
 *	[-Idirectory] Insert directory into the search path for #include files.
 *	[-Pcpp] Use cpp as the C preprocessor.  By default /usr/lib/cpp
 *		is used (utask_Dir:gnu_cpp on a vms machine)
 *
 *      Note that the -D, -U and -I options are passed directly to the C
 *	preprocessor.  If -n is set, none of the other options have any 
 *	effect (and C preprocessor statements may call errors)	.
 *
 *	
 * See Also:
 *	cpp(1), Sds manual.
 *
 * Support : Tony Farrell, AAO
 *-
 * History:
 *	16-Jul-1992 - TJF - Original version.
 *	14-Apr-1994 - TJF - Support DECC on VMS
 *      23-Feb-1995 - TJF - Setup comments for man.
 *	03-Mar-1995 - TJF- Add -W option and change from IntAsLong to 
 *				IntAs32Bit
 *      06-Nov-1997 - TJF - Add -f option, -t, -T and -N options.
 *      18-Mar-2003 - TJF - Fix some compilation warnigns.
 *      18-Jun-2003 - TJF - getopt should only be prototyped if DPOSIX_1
 *                          is not defined.
 *      11-Aug-2003 - TJF - The above change requires we include drama.h
 *      05-Apr-2004 - TJF - Rename yyparse to SdsYyparse().
 *      04-Jun-2012 - TJF - Fix printf of non-literal string picked up
 *                          as potential security issue by CLang compiler 
 *      04-Apr-2018 - TJF - Add -c option, and assoicated SdsIntCompatibity flag.
 *       
 *
 */


static const char *rcsId="@(#) $Id: ACMM:sds/sdsc.c,v 3.94 09-Dec-2020 17:15:25+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);

/* This is needed to get solaris include files to add extra stuff */
#define __EXTENSIONS__ 1

#include "sdscompiler.h"
#include "drama.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#if defined(VMS) || defined(__VMS)
#    include <ssdef.h>
#    include <processes.h>
#    include <unixio.h>
#elif defined(WIN32)
#    include <windows.h>
#else
#    include <sys/types.h>
#    include <sys/wait.h>
#    include <unistd.h>
#endif
/*
 *	Function prototypes
 */

#ifndef DPOSIX_1
    DPUBLIC int getopt(int argc, char **argv, char *optstring);
#endif
int SdsYyparse(void);
#if defined(sun) &&  !defined(solaris2) 
    int fflush(FILE *);
    int pipe(int fd[]);
    int fork(void);
    int dup2(int fd1, int fd2);
 /*   void exit(int); */
    int execve(char *path, char *argv[], char *env[]);
    int unlink (char *);
    int wait( int *);
#endif
#ifdef DECSTATION
#   include <sys/types.h>
#   include <unistd.h>
    void exit(int);
    int fork(void);
    int execve(char *path, char *argv[], char *env[]);
    int wait( int *);
    int unlink (char *);
    int pipe(int fd[]);
    int dup2(int fd1, int fd2);
#endif

/*
 *  If defined true, then a temporary file is used to store the output of cpp.
 *  If defined false, then pipes will be used (which don't work as yet)
 */
#define TEMPFILE 1

int main(int argc, char *argv[])
{
    int c;			/* Next option			*/
    extern int optind;		/* Option index			*/
    extern char *optarg;	/* Option argument		*/
    char *cpp = DEFAULTCPP;	/* C preprocessor 		*/
    char *cppargs[100];		/* Arguments to cpp		*/
    int  cppcnt=0;		/* Number of arguments to cpp 	*/
    int i;			/* Scratch			*/

    int verify = 0;		/* Output cpp comment line	*/
    int errflg = 0;		/* Option error occured		*/
    int nocpp = 0;		/* Don't run cpp		*/
    int writesds = 0;		/* Write an sds file, not code */
    
    char *infile = "<stdin>";	/* Input file name		*/
    char *outfile = "<stdout>"; /* Output file name		*/

    SdsOutputMessages = SDS_COMP_MESS_ERROR; /* Enable the output of messages */
/*
 *   Setup cpp arguments to define macros
 */
    cppargs[cppcnt++] = cpp;
    cppargs[cppcnt++] = "-DSDS";
    cppargs[cppcnt++] = "-D___SDS___";
/*
 *   Examine options
 */
    while (( c= getopt (argc, argv, "nvlswcWTI:D:U:P:f:t:N:")) != EOF)
    {
	switch (c)
	{
            case 'I':	/* Add an include file search path	*/
		cppargs[cppcnt++] = argv[optind-1];
		break;
            case 'D':	/* Add a define				*/
		cppargs[cppcnt++] = argv[optind-1];
	        break;
            case 'U':	/* Add a Undefine			*/
		cppargs[cppcnt++] = argv[optind-1];
		break;
	    case 'P':	/* Select the preprocessor to be used	*/
		printf("C preprocessor %s\n",optarg);
		cpp = optarg;
		cppargs[0] = cpp;
		break;
            case 'f':   /* Set function name                    */
                SdsFunctionName = optarg;
                break;
            case 't':   /* Type of variable to be declared      */
                SdsDeclareType = optarg;
                break;
            case 'N':   /* Name of variable to be declared      */
                SdsVarName = optarg;
                break;
            case 'v':	/* Output cpp command line		*/
		verify = 1;
		break;
            case 'c':   /* Enable compatiliby with source files */
                SdsIntCompatibility = 1;
                break;
	    case 'n':	/* Don't run cpp			*/
		nocpp = 1;
		break;
	    case 'l':
		SdsIntAs32Bit = 1;
	        break;
	    case 's':
		SdsIntAs32Bit = 0;
		break;
	    case 'w':
		writesds = 1;
		break;
	    case 'W':
                SdsOutputMessages |= SDS_COMP_MESS_WARN;
	        break;
            case 'T':
                SdsGenerateTcl = 1;
                break;
	    default:
		errflg++;
	}
    }
    if (errflg)	
	exit(-1);	/* getopt has output a message		*/
/*
 *  If an input file is specified - if not using cpp, just make the file
 *	the input file.  If using cpp, add the input file to the argument
 *	list.
 */
    if (argc > optind)
    {
	infile = argv[optind];
	if (nocpp)
	{
	    if (freopen(argv[optind],"r",stdin) == NULL)
	    {
	        fprintf(stderr,"Unable to open input file %s\n",argv[optind]);
		exit(-2);
	    }
        }
	else
	    cppargs[cppcnt++] = argv[optind];
        optind++;
    }
#   if defined(VMS) || defined(__VMS)
    else
    {
	fprintf(stderr,"You must specify an input file with VMS\n");
	exit(-2);
    }
#   endif

/*
 *  If writeing sds, we need an output file.
 */
    if (writesds)
    {
	if (argc > optind)
	    outfile = argv[optind]; 
	else
        {
	    fprintf(stderr,"You must specify an output file with -w\n");
	    exit(-2);
        }
    }
/*
 *      If an output file is specified, subsitute it for stdout.
 */
    else
    {
        if (argc > optind)
        {
	    outfile = argv[optind]; 
	    if (freopen(argv[optind],"w",stdout) == NULL)
	    {
	        fprintf(stderr,"Unable to open output file %s\n",argv[optind]);
	        exit(-2);
	    }
        }
    }
/*
 *  Output the cpp command to stdout if required.
 */
    if ((verify)&&(!nocpp))
    {
        printf("%s",cpp);
        printf(" ");
        for (i = 0 ; i <  cppcnt ; ++ i)
        {
	    printf("%s",cppargs[i]);
	    printf(" ");
        }
        printf("\n");
    }
/*
 * If not running cpp, then stdin points to the input file.
 * If running cpp, then either stdin points to the input file or it
 *  is the first argument.
 *
 * Stdout is correct.
 */
    strcpy(SdsFileName,infile);		/* Save filename for messages */
    SdsDims = 0;			/* Clear the number of dims	*/
    SdsParsingError = SDS__OK;		/* Clear the Parser error flag */
    SdsOutputCode = !writesds;		/* Enable the output of code */
					/* unless writing sds	     */
    if (nocpp)
	SdsYyparse();			/* Parse the input file	*/
    else if (TEMPFILE)
    {
/*
 *	Using temp file for cpp output.
 */
	int stat;
	char tempname[L_tmpnam];
	tmpnam(tempname);		/* Get a temp file name	*/

        /*fprintf(stderr,"CCP writing to file %s\n", tempname);*/
#ifdef WIN32
	{
	  STARTUPINFO    si;
	  PROCESS_INFORMATION  pi;

	  char * cpp_cmd;
	  int i=0, ls=0;

	  cppargs[cppcnt++] = "-o";	/* Put -o tempname in cpp arg list */
	  cppargs[cppcnt++] = tempname;

	  /* Now build the argument list into a single string as needed by Win32 */
	  for ( i=0; i<cppcnt; i++ ) ls += strlen( cppargs[i] ) + 1;
	  cpp_cmd = (char *) malloc( ls );
	  strcpy( cpp_cmd, cppargs[0] );
	  for ( i=1; i<cppcnt; i++ )
	  {
	      strcat( cpp_cmd, " " );
	      strcat( cpp_cmd, cppargs[i] );
	  }

	  GetStartupInfo(&si);
	  CreateProcess(NULL, cpp_cmd, /* Name of app to launch */
			NULL,        /* Default process security attributes */
			NULL,        /* Default thread security attributes */
			FALSE,       /* Don't inherit handles from the parent */
			0,           /* Normal priority */
			NULL,        /* Use the same environment as the parent */
			NULL,        /* Launch in the current directory */
			&si,         /* Startup Information */
			&pi);        /* Process information stored upon return */

	  /* Suspend our execution until the child has terminated. */
	  WaitForSingleObject(pi.hProcess, INFINITE);

          /* Obtain the termination code. */
	  GetExitCodeProcess(pi.hProcess, &stat);
	}
#else
#       if defined(VMS) || defined(__VMS)
	    if (vfork() == 0)
	    {
                cppargs[cppcnt++] = tempname;	/* Put tempname in cpp arg list */
                cppargs[cppcnt++] = 0;	/* Zero end of list	*/
	        execve(cpp,cppargs,0);	/* Run cpp		*/ 
		
	    }
#       else
	    if (fork() == 0)		
	    {				/* Child, runs cpp	*/
                cppargs[cppcnt++] = 0;	/* Zero end of list	*/
					/* Make the temp file the output */
	        if (freopen(tempname,"w",stdout) == NULL)
	        {
		    fprintf(stderr,"Unable to open temp file %s for output\n",
						tempname);
		    exit(-1);
                }
	        errno = 0;
	        execve(cpp,cppargs,0);	/* Run cpp		*/ 
	        fprintf(stderr,"Unable to exec %s, errno = %d\n",cpp,errno);
   	    }
#       endif
	wait(&stat);			/* Wait for cpp to complete	*/
#endif /* ! defined WIN32 */
	if (freopen(tempname,"r",stdin) == NULL)	/* Make temp the input*/
	{
	    fprintf(stderr,"Unable to open temp file %s for input\n",tempname);
	    exit(-1);
        }
/*
        fprintf(stderr," About to invoke SdsYyparse() with file %s\n",
                tempname);
*/
	SdsYyparse();				/* Parse the temp file */
/*
        fprintf(stderr,"parse complete\n");
*/
#       if defined(VMS) || defined(__VMS)
	    strcat(tempname,".;0");
	    delete(tempname);
#	else
	    unlink(tempname);			/* Remove tempfile	*/
#       endif
	
    }
/*
 *	Using pipes.  This currently hangs.
 */
#if !defined(WIN32) && !(defined(VMS)) && !(defined(__VMS))
    else 
    {
        int p[2];
        cppargs[cppcnt++] = 0;	/* Zero end of list	*/
	pipe(p);		/* Get a pipe for feeding the output of cpp */
				/* to this program		*/
        if (fork() == 0)	/* Fork				*/
	{			/* This is the child, it will run cpp	*/
	    fprintf(stderr,"Forked ccp code\n");
	    dup2(p[1],1);	/* Replace output with pipe output */
	    execve(cpp,cppargs,0); 
	    fprintf(stderr,"Unable to exec %s\n",cpp);
	}
	else			/* This is the parent, it run SdscMain	*/
	{
	    /*int c;*/
	    fprintf(stderr,"Forked Sdsc code\n");
	    dup2(p[0],0);	/* Replace input with pipe input	*/
            printf("SDSC started output\n");
	    SdsYyparse();		/* Parse the output of Cpp		*/
		/*
	    while ((c = getchar()) != EOF)
		putchar(c);
		*/
	    printf("SDSC finished\n");
	}
	
    }
#endif

    if (writesds) 
        SdsWrite(SdsId,outfile,&SdsParsingError);

    if (SdsParsingError == SDS__NOINPUT)
	SdsParsingError = 0;
    if (SdsParsingError)
    {
	fprintf(stderr,"Parsing failed, first error was %d\n",SdsParsingError);
	return(SdsParsingError);
    }
    else
    {
#       if defined(VMS) || defined(__VMS)
	    return(SS$_NORMAL);
#       else
	    return(0);
#       endif
    }
}
