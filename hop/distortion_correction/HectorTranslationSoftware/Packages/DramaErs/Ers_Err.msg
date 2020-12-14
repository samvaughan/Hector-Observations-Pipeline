! "@(#) $Id: ACMM:DramaErs/Ers_Err.msg,v 3.19 09-Dec-2020 17:17:02+11 ks $"
.TITLE   ERS_ERR  Error reporting system
.FACILITY ERS,1827/PREFIX=ERS__
.SEVERITY ERROR
.BASE 2
NOSPACE <The ERS system error table is full>
ACTIVE <The ERS system is already active>
NOTACTIVE <The ERS system is not active>
TOOMANPOPS <ErsPop was called more times then ErsPush>
TASKVARERR <Error adding or deleting task a variable>
MALLOCERR <Malloc error>
INITIALISED <Ers already initialised>
NOTINITIALISED <Ers not initialised>
FLUSHING <Operation not permissible while flushing>
INTHAND <Operation (ErsClear/ErsFlush) is not permissible while ERS is in interrupt mode>
.END
