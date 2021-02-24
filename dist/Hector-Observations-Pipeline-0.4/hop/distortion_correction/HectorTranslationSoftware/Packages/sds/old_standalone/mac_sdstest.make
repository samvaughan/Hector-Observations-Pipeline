#
# "@(#) $Id: ACMM:sds/old_standalone/mac_sdstest.make,v 3.94 09-Dec-2020 17:15:53+11 ks $"
#   File:       sdstest.make
#   Target:     sdstest
#   Sources:    sds.c sdsutil.c sdstest.c
#   Created:    Friday, February 28, 1992 4:40:17 PM

sds.c.o Ä sdstest.make sds.c
	C -dMPW sds.c
sdsutil.c.o Ä sdstest.make sdsutil.c
	C sdsutil.c
sdstest.c.o Ä sdstest.make sdstest.c
	C sdstest.c
sdstest ÄÄ sdstest.make sds.c.o sdsutil.c.o sdstest.c.o
	Link -w -t MPST -c 'MPS ' ¶
		sds.c.o ¶
		sdsutil.c.o ¶
		sdstest.c.o ¶
		"{Libraries}"Interface.o ¶
		"{CLibraries}"CRuntime.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{CLibraries}"CSANELib.o ¶
		"{CLibraries}"Math.o ¶
		"{CLibraries}"CInterface.o ¶
		"{Libraries}"ToolLibs.o ¶
		-o sdstest
