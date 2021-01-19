/*
 * Program to test the ERS functions ErsSPrintf() and ErsVSPrintf().
 */


static const char *rcsId="@(#) $Id: ACMM:DramaErs/sprintftest.c,v 3.19 09-Dec-2020 17:17:02+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "Ers.h"


static int  TestSPrintf(const int maxLength, 
			char *string, 
			const char * fmt,...)
#ifdef __GNUC__
	__attribute__ ((format (printf, 3, 4)))
#endif
        ;


int main()
{
#define BUFLEN 21
#define TESTLEN 10
    char buffer[BUFLEN];

    const char *testString = "12345678901234567890";  /* 20 characters long */
    int result = 0;
    /*
     * First a basic coding check - is the test string the length we expect?
     */
    int len = strlen(testString);
    if (len != 20)
    {
        fprintf(stderr,"Test string is too long - %d rather then 20. Programming error\n", len);
        exit(1);
    }

    assert(TESTLEN < BUFLEN);
    assert(len < BUFLEN);
    /*
     *	So TESTLEN is much shorter.  This should give us an error.
     */
    result = ErsSPrintf(TESTLEN, buffer, "%s", testString);

    if (result != EOF)
    {
        fprintf(stderr,"ErsSPrintf() test 1 return wrong result, returned %d, should have returned %d\n",
                result, EOF);
        exit(1);
    }


    printf("ErsSPrintf result 1 correct\n");

    /*
     * len at this point is the length of the string - an edge case so
     * we check it.
     */
    result = ErsSPrintf(len, buffer, "%s", testString);

    if (result != EOF)
    {
        fprintf(stderr,"ErsSPrintf() test 2 return wrong result, returned %d, should have returned %d\n",
                result, EOF);
        exit(1);
    }
    printf("ErsSPrintf result 2 correct\n");


    /*
     * Now use the full buffer.
     */
    result = ErsSPrintf(BUFLEN, buffer, "%s", testString);

    if (result != len)
    {
        fprintf(stderr,"ErsSPrintf() test 3 return wrong result, returned %d, should have returned %d\n",
                result, len);
        exit(1);
    }
    if (strcmp(buffer, testString) != 0)
    {
        fprintf(stderr,"ErsSPrintf() test 3 format error, expected \"%s\", got \"%s\"\n",
                testString, buffer);
        exit(1);
    }

    printf("ErsSPrintf result 3 correct\n");


    
    /*
     * Test of formatting an integer.  There are too many types to try
     * them all, but we will do this basic pair.
     */
    result = ErsSPrintf(2, buffer, "%d", 12345);

    if (result != EOF)
    {
        fprintf(stderr,"ErsSPrintf() test 4 return wrong result, returned %d, should have returned %d\n",
                result, EOF);
        exit(1);
    }
    printf("ErsSPrintf result 4 correct\n");

    result = ErsSPrintf(BUFLEN, buffer, "%d", 12345);

    if (result != 5 /* Number of numbers in the integer */)
    {
        fprintf(stderr,"ErsSPrintf() test 4 return wrong result, returned %d, should have returned %d\n",
                result, 5 /* Number of numbers in the integer */);
        exit(1);
    }
    printf("ErsSPrintf result 5 correct\n");



    /*
     * Now run our own version of SPrintf(), repeating each of the
     * above.  This is about ensuring that ErsVSPrintf() works.
     */
    /*
     *	So TESTLEN is much shorter.  This should give us an error.
     */
    result = TestSPrintf(TESTLEN, buffer, "%s", testString);

    if (result != EOF)
    {
        fprintf(stderr,"TestSPrintf() test 1 return wrong result, returned %d, should have returned %d\n",
                result, EOF);
        exit(1);
    }


    printf("ErsVSPrintf result 1 correct\n");

    /*
     * len at this point is the length of the string - an edge case so
     * we check it.
     */
    result = TestSPrintf(len, buffer, "%s", testString);

    if (result != EOF)
    {
        fprintf(stderr,"TestSPrintf() test 2 return wrong result, returned %d, should have returned %d\n",
                result, EOF);
        exit(1);
    }
    printf("ErsVSPrintf result 2 correct\n");


    /*
     * Now use the full buffer.
     */
    result = TestSPrintf(BUFLEN, buffer, "%s", testString);

    if (result != len)
    {
        fprintf(stderr,"TestSPrintf() test 3 return wrong result, returned %d, should have returned %d\n",
                result, len);
        exit(1);
    }
    if (strcmp(buffer, testString) != 0)
    {
        fprintf(stderr,"TestSPrintf() test 3 format error, expected \"%s\", got \"%s\"\n",
                testString, buffer);
        exit(1);
    }

    printf("ErsVSPrintf result 3 correct\n");


    
    /*
     * Test of formatting an integer.  There are too many types to try
     * them all, but we will do this basic pair.
     */
    result = TestSPrintf(2, buffer, "%d", 12345);

    if (result != EOF)
    {
        fprintf(stderr,"TestSPrintf() test 4 return wrong result, returned %d, should have returned %d\n",
                result, EOF);
        exit(1);
    }
    printf("ErsVSPrintf result 4 correct\n");

    result = TestSPrintf(BUFLEN, buffer, "%d", 12345);

    if (result != 5 /* Number of numbers in the integer */)
    {
        fprintf(stderr,"TestSPrintf() test 4 return wrong result, returned %d, should have returned %d\n",
                result, 5 /* Number of numbers in the integer */);
        exit(1);
    }
    printf("ErsVSPrintf result 5 correct\n");





    return 0;
}

/*
 * Our own implementation of ErsSPrint(), so we can test ErsVSPrintf()
 * directly
 */
static int TestSPrintf(const int maxLength, 
                       char *string, 
                       const char * fmt,...)
{
    va_list args;
    va_start(args,fmt);
    
    return(ErsVSPrintf(maxLength,string,fmt,args));

}
