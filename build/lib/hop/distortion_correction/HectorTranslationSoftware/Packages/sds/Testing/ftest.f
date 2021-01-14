      program ftest

*     @(#) $Id: ACMM:sds/Testing/ftest.f,v 3.94 09-Dec-2020 17:15:54+11 ks $
*
*     Example of usage of fortran interface to SDS and ARG.
*    

      integer status
      integer ignore
      integer id

*    Initialise status to zero, otherwise calls won't work.
      status = 0
      
*    Initialise ID to zero, so we know if we need to tidy it up.
      id = 0

*     Use the arg routines to create an SDS structure. For simple
*     structures this is simpler then using SDS directly, but does require
*     we link against DRAMA's ERS library.

      call arg_new(id, status)

*     Create a couple of components in the structure.  The arg_put calls
*     just wrap up calls to sds_new for simple cases.
      call arg_put0i(id, 'Integer', 1, status)

*     Note that if you us arg_put0d here, you must ensure the value
*     passed is "DOUBLE" rather then the default of "REAL."
      call arg_put0r(id, 'Real', 1.2, status)


*     Call the SDS listing routine.  This will output the structure to stdout.
      call sds_list(id, status)

*     If the structure was created (by arg_new) then we must tidy up,
*     even if status was set bad by a later call.
      if (id .ne. 0) then
*        We could just use arg_delete here, but I use the sds calls to
*        show how to use them.  arg_delete just calls these two routines.
         ignore = 0
         call sds_delete(id, ignore)
         call sds_free_id(id, ignore)
      endif

*     Flush any errors.
      if (status .ne. 0) then
          call err_flush(status)
      endif


      end
