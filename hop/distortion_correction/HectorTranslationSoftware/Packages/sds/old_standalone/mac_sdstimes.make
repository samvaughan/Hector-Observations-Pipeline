
# "@(#) $Id: ACMM:sds/old_standalone/mac_sdstimes.make,v 3.94 09-Dec-2020 17:15:51+11 ks $"
#
#   File:       sdstimes.make
#   Target:     sdstimes
#   Sources:    sds.c timer.c sdstimes.c
#   Created:    Friday, February 28, 1992 4:32:40 PM

sds.c.o Ä sdstimes.make sds.c
	C -dMPW sds.c
timer.c.o Ä sdstimes.make timer.c
	C timer.c
sdstimes.c.o Ä sdstimes.make sdstimes.c
	C -dMPW sdstimes.c
sdstimes ÄÄ sdstimes.make sds.c.o timer.c.o sdstimes.c.o
	Link -w -t MPST -c 'MPS ' ¶
		sds.c.o ¶
		timer.c.o ¶
		sdstimes.c.o ¶
		"{Libraries}"Interface.o ¶
		"{CLibraries}"CRuntime.o ¶
		"{CLibraries}"StdCLib.o ¶
		"{CLibraries}"CSANELib.o ¶
		"{CLibraries}"Math.o ¶
		"{CLibraries}"CInterface.o ¶
		"{Libraries}"ToolLibs.o ¶
		-o sdstimes
