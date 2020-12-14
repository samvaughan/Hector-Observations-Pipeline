#
# "@(#) $Id: ACMM:sds/old_standalone/mac_readtest.make,v 3.94 09-Dec-2020 17:15:39+11 ks $"
#
#   File:       readtest.make
#   Target:     readtest
#   Sources:    sds.c sdsutil.c readtest.c
#   Created:    Friday, February 28, 1992 4:27:46 PM

sds.c.o Ä readtest.make sds.c
	C -dMPW sds.c
sdsutil.c.o Ä readtest.make sdsutil.c
	C sdsutil.c
readtest.c.o Ä readtest.make readtest.c
	C readtest.c
readtest ÄÄ readtest.make sds.c.o sdsutil.c.o readtest.c.o
	Link -w -t MPST -c 'MPS ' ¶
		sds.c.o ¶
		sdsutil.c.o ¶
		readtest.c.o ¶
		"{Libraries}"Interface.o ¶
		"{CLibraries}"CRuntime.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{CLibraries}"CSANELib.o ¶
		"{CLibraries}"Math.o ¶
		"{CLibraries}"CInterface.o ¶
		"{Libraries}"ToolLibs.o ¶
		-o readtest
