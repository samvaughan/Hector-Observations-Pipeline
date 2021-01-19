# "@(#) $Id: ACMM:slalib_o/README.txt,v 1.6 06-Apr-2017 11:44:53+10 mnb $"
----------------------------------------------------------------
Note - SLALIB C interface is proprietary software. Use slalib_o for
distribution outside the AAO.

But also note that new code should probably use the SOFA library:

    http://www.iausofa.org/index.html 

This contains most or all of the functionality of SLALIB with a more
open licence.

----------------------------------------------------------------

Pat Wallac's SLA library C source - obfuscated version.  AAO is allowed 
to use this being an Observatory.    Pat's latest copy can be found at

    ftp://ftp.starlink.rl.ac.uk/pub/users-ftp/ptw/slalib/slalib_c.tar.Z


But it is normally kept non-readable, you have to arrange with Pat to have
the protection temporarly disabled.  Pat's e-mail addresses
as of 12-Jul-2007

   ptw@star.rl.ac.uk        day-job, astronomical things
   P.T.Wallace@rl.ac.uk     day-job, boring stuff
   ptw@tpsoft.demon.co.uk   moonlighting pursuits

The obfuscated version of slalib can be used for stuff we might distribute
on the web.  See the ACMM module "slalib" for the standard version.

Conditions of use:

 SLALIB/C is proprietary software.  Your free-of-charge personal
 license is provided on an understanding of non-disclosure to third
 parties, and covers non-profit astronomy research applications only.
 Though you may make the software available to immediate colleagues in
 object form (for example as compiled applications or as a linkable
 library), you must not place the source code where copies can be made
 by others who may assume it is freeware.  In particular, it must not
 become visible on the Web, whether as text files or in archived form.

 If addition, for the obfuscated version you have permission for a 
 low-profile inclusion of only the functions you need in your application 
 releases.

Standard disclaimer:  use at your own risk.

The library is protected by copyright and is strictly for non-profit
use except by special arrangement.  Please don't give copies to anyone
else (tell them to contact me directly) and don't let any of the source
code appear on the Internet in any form.

The SLALIB/C manual is called sla_A4.pdf or sla_US.pdf depending which
stationery you wish to use.

In June 2005, the UK Particle Physics and Astronomy Research Council
withdrew funding from Starlink.  Support for Starlink software
formally ceased at that date.  SLALIB/C etc. is a special case as it
is (and always was in fact) proprietary and is supported privately by
its author.

-------

This software has made use of the ESO cmm system (known internally
at AAO as "acmm").  

This module is a NOTAGCHK (No Tag Check) module - would normally be driven
by a Makefile.
