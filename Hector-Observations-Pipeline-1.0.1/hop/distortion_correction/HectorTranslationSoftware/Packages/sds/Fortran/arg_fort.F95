!
! Additional Fortran ARG_xxx routines.
! These call the routines in arg_c.c
!

! --------------------------------------------------------------

SUBROUTINE ARG_GETDI(ID,NAME,DIVALUE,IVALUE,STATUS)
!+
!  Purpose:
!     Gets an integer item value from an argument structure. Uses a
!     default value if the item is not present.
!
!  Description:
!     Implements default value handling not present in ARG routines.
!
!  Arguments:
!     ID = INTEGER (Given)
!        SDS object identifier
!     NAME = CHARACTER*(*) (Given)
!        Item name
!     DIVALUE = INTEGER (Given)
!        Default item value
!     IVALUE = INTEGER (Returned)
!        Item value
!     STATUS = INTEGER (Given and Returned)
!        Global status
!-
   IMPLICIT NONE

   ! arguments
   INTEGER ID    
   CHARACTER*(*) NAME      
   INTEGER DIVALUE,IVALUE
   INTEGER STATUS      

   IF (STATUS.NE.0) RETURN 
   CALL ARG_GET0I(ID,NAME,IVALUE,STATUS)
   IF (STATUS.NE.0) THEN
      IVALUE = DIVALUE
      STATUS = 0
   ENDIF
END SUBROUTINE ARG_GETDI

! --------------------------------------------------------------

SUBROUTINE ARG_GETDR(ID,NAME,DRVALUE,RVALUE,STATUS)
!+
!  Purpose:
!     Gets a real item value from an argument structure. Uses a
!     default value if the item is not present.
!
!  Description:
!     Implements default value handling not present in ARG routines.
!
!  Arguments:
!     ID = INTEGER (Given)
!        SDS object identifier
!     NAME = CHARACTER*(*) (Given)
!        Item name
!     DRVALUE = REAL (Given)
!        Default item value
!     RVALUE = REAL (Returned)
!        Item value
!     STATUS = INTEGER (Given and Returned)
!        Global status
!-
   IMPLICIT NONE

   ! arguments
   INTEGER ID
   CHARACTER*(*) NAME      
   REAL DRVALUE,RVALUE
   INTEGER STATUS      

   IF (STATUS.NE.0) RETURN 
   CALL ARG_GET0R(ID,NAME,RVALUE,STATUS)
   IF (STATUS.NE.0) THEN
      RVALUE = DRVALUE
      STATUS = 0
   ENDIF
END SUBROUTINE ARG_GETDR

! --------------------------------------------------------------

SUBROUTINE ARG_GETDD(ID,NAME,DDVALUE,DVALUE,STATUS)
!+
!  Purpose:
!     Gets a double precision item value from an argument structure. Uses a
!     default value if the item is not present.
!
!  Description:
!     Implements default value handling not present in ARG routines.
!
!  Arguments:
!     ID = INTEGER (Given)
!        SDS object identifier
!     NAME = CHARACTER*(*) (Given)
!        Item name
!     DDVALUE = DOUBLE PRECISION (Given)
!        Default item value
!     DVALUE = DOUBLE PRECISION (Returned)
!        Item value
!     STATUS = INTEGER (Given and Returned)
!        Global status
!-
   IMPLICIT NONE

   ! arguments
   INTEGER ID
   CHARACTER*(*) NAME      
   DOUBLE PRECISION DDVALUE,DVALUE
   INTEGER STATUS      

   IF (STATUS.NE.0) RETURN 
   CALL ARG_GET0D(ID,NAME,DVALUE,STATUS)
   IF (STATUS.NE.0) THEN
      DVALUE = DDVALUE
      STATUS = 0
   ENDIF
END SUBROUTINE ARG_GETDD

! --------------------------------------------------------------

SUBROUTINE ARG_GETDC(ID,NAME,DCVALUE,CVALUE,STATUS)
!+
!  Purpose:
!     Gets a character string item value from an argument structure. Uses a
!     default value if the item is not present.
!
!  Description:
!     Implements default value handling not present in ARG routines.
!
!  Arguments:
!     ID = INTEGER (Given)
!        SDS object identifier
!     NAME = CHARACTER*(*) (Given)
!        Item name
!     DCVALUE = CHARACTER VECTOR*(*) (Given)
!        Default item value
!     CVALUE = CHARACTER VECTOR*(*) (Returned)
!        Item value
!     STATUS = INTEGER (Given and Returned)
!        Global status
!-
   IMPLICIT NONE

   ! arguments
   INTEGER ID     
   CHARACTER*(*) NAME      
   CHARACTER*(*) CVALUE     
   CHARACTER*(*) DCVALUE     
   INTEGER STATUS      
   
   IF (STATUS.NE.0) RETURN 
   CALL ARG_GET0C(ID,NAME,CVALUE,STATUS)
   IF (STATUS.NE.0) THEN
      CVALUE = DCVALUE
      STATUS = 0
   ENDIF
END SUBROUTINE ARG_GETDC

! --------------------------------------------------------------

SUBROUTINE ARG_GET0L(ID,NAME,LVALUE,STATUS)
!+
!  Purpose:
!     Gets a logical item value from an argument structure.
!
!  Description:
!     Implements Fortran logical value handling using the ARG integer routine.
!     Returns error if item is not present.
!
!  Arguments:
!     ID = INTEGER (Given)
!        SDS object identifier
!     NAME = CHARACTER*(*) (Given)
!        Item name
!     LVALUE = LOGICAL (Returned)
!        Item value
!     STATUS = INTEGER (Given and Returned)
!        Global status
!-
   IMPLICIT NONE

   ! arguments
   INTEGER ID
   CHARACTER*(*) NAME      
   LOGICAL LVALUE
   INTEGER STATUS      

   ! local variables
   INTEGER IVALUE
   
   IF (STATUS.NE.0) RETURN 
   CALL ARG_GET0I(ID,NAME,IVALUE,STATUS)
   IF (IVALUE.EQ.0) THEN 
      LVALUE = .FALSE.
   ELSE
      LVALUE = .TRUE.
   ENDIF
END SUBROUTINE ARG_GET0L

! --------------------------------------------------------------

SUBROUTINE ARG_GETDL(ID,NAME,DLVALUE,LVALUE,STATUS)
!+
!  Purpose:
!     Gets a logical item value from an argument structure. Uses a
!     default value if the item is not present.
!
!  Description:
!     Implements Fortran logical value handling using the ARG integer routine,
!     and default value handling not present in ARG routines.
!
!  Arguments:
!     ID = INTEGER (Given)
!        SDS object identifier
!     NAME = CHARACTER*(*) (Given)
!        Item name
!     DLVALUE = LOGICAL (Given)
!        Default item value
!     LVALUE = LOGICAL (Returned)
!        Item value
!     STATUS = INTEGER (Given and Returned)
!        Global status
!-
   IMPLICIT NONE

   ! arguments
   INTEGER ID    
   CHARACTER*(*) NAME      
   LOGICAL DLVALUE,LVALUE
   INTEGER STATUS      
   
   ! local variables
   INTEGER IVALUE
   
   IF (STATUS.NE.0) RETURN 
   CALL ARG_GET0I(ID,NAME,IVALUE,STATUS)
   IF (STATUS.NE.0) THEN
      LVALUE = DLVALUE
      STATUS = 0
   ELSE
      IF (IVALUE.EQ.0) THEN
         LVALUE = .FALSE.
      ELSE
         LVALUE = .TRUE.
      ENDIF
   ENDIF
END SUBROUTINE ARG_GETDL

! --------------------------------------------------------------

SUBROUTINE ARG_PUT0L(ID,NAME,LVALUE,STATUS)
!+
!  Purpose:
!     Puts a logical value into an argument structure.
!
!  Description:
!     Implements Fortran logical value handling using the ARG integer routine.
!
!  Arguments:
!     ID = INTEGER (Given)
!        SDS object identifier
!     NAME = CHARACTER*(*) (Given)
!        Item name
!     LVALUE = LOGICAL (Given)
!        Item value
!     STATUS = INTEGER (Given and Returned)
!        Global status
!-
   IMPLICIT NONE

   ! arguments
   INTEGER ID    
   CHARACTER*(*) NAME      
   LOGICAL LVALUE
   INTEGER STATUS      

   ! local variables
   INTEGER IVALUE
      
   IF (STATUS.NE.0) RETURN 
   IF (LVALUE) THEN 
      IVALUE = 1
   ELSE
      IVALUE = 0
   ENDIF
   CALL ARG_PUT0I(ID,NAME,IVALUE,STATUS)
END SUBROUTINE ARG_PUT0L
