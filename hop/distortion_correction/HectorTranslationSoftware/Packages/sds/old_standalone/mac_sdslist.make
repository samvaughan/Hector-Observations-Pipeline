#
# "@(#) $Id: ACMM:sds/old_standalone/mac_sdslist.make,v 3.94 09-Dec-2020 17:15:38+11 ks $"
#
#   File:       sdslist.make
#   Target:     sdslist
#   Sources:    sds.c sdsutil.c sdslist.c
#   Created:    Friday, February 28, 1992 4:24:25 PM

sds.c.o Ä sdslist.make sds.c
	C -dMPW sds.c
sdsutil.c.o Ä sdslist.make sdsutil.c
	C sdsutil.c
sdslist.c.o Ä sdslist.make sdslist.c
	C sdslist.c
sdslist ÄÄ sdslist.make sds.c.o sdsutil.c.o sdslist.c.o
	Link -w -t MPST -c 'MPS ' ¶
		sds.c.o ¶
		sdsutil.c.o ¶
		sdslist.c.o ¶
		"{Libraries}"Interface.o ¶
		"{CLibraries}"CRuntime.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{CLibraries}"CSANELib.o ¶
		"{CLibraries}"Math.o ¶
		"{CLibraries}"CInterface.o ¶
		"{Libraries}"ToolLibs.o ¶
		-o sdslist
