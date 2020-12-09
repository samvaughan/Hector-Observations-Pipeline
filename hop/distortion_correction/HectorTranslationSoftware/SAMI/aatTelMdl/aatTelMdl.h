#ifndef _AAT_TELMDL_H
#define _AAT_TELMDL_H
/*
 *+                     a a t T e l M d l . h

 * Include File name:
      aatTelMdl.h

 * Function:
       AAT Telescope Model software - header.

 * Description:
       A header file for classes used in the conversion of RA/Dec positions to
       AAT focal plane positions.  All are within the "AAT" namespace.


       Structs => telMdlPM_PARS.  Used to specify proper motion parameters.
               => telMdlPARS.     Contains an array of doubles representing
                                  the telescope model parameters.
                  telMdl.         The telescope model class.  Implements methods
                                  for the required conversions, writing to/reading
                                  from files and functions required to help
                                  calibrate the module.

                  telMdlMean.     Subclass of telMdl which provides for
                                  conversions from Mean sky positions to
                                  the telescope model rather then
                                  just apparent sky positions.

 * Language:
      C++


 * Support: Tony Farrell, AAO
  *-

 * @(#) $Id: ACMM:aatTelMdl/aatTelMdl.h,v 1.10 02-Nov-2016 11:09:55+11 tjf $

 * History:
	 12-May-2006 - TJF - Original version, based on echidna model.
         20-May-2011 - TJF - Renamed from cassTelMdl to aatTelMdl and now supports
                             prime focus triplet.
         07-Dec-2011 - TJF - Extracted from ACMM module 2dFctlrTask into 
                             new module aatTelMdl.
         04-May-2012 - TJF - Add  AAT_TELMDL_USE_PF_ORIG_VAL macro and warning.
         27-Sep-2012 - TJF - Add _logInit, _noRefraction and _airMassLimit static 
                              variables and methods to set them.
     12-Jul-2013 - TJF - Add KOALA variations of F/8 top-end

 *  Copyright (c) Anglo-Australian Telescope Board, 2006.
    Not to be used for commercial purposes without AATB permission.

 */

#include <fstream>

extern "C"
{
#include "slalib.h"
}
#include "status.h"
#include "field_properties.h"

/*
 * This triggers the code to be built using the original focal lenght value.  We use
 * it to ensure the warning below is output if we do so.
 *
 * NOTE - Recompile and re-release this code module if changing the value of this
 * macro, don't just change it in the this header file.
 */
//#define AAT_TELMDL_USE_PF_ORIG_VAL 1
#ifdef AAT_TELMDL_USE_PF_ORIG_VAL
#warning "PF Focal length set to original rather then optimized value"
#endif

namespace AAT 
{
/* 
 * This structure contains parameters used for proper motion calcuations.
 */

    struct  telMdlPM_PARS {
        double ra;       /* Proper motion - RA, changes per Julian Year (radians)*/
        double dec;      /* Proper motion - Dec, changes per Juilian Year */
        double parallax; /* Parallax (arcsec) */
        double radialVel;/* Radial velocity (km/s, +ve if receding) */

        /*
         * constructor
         */
        telMdlPM_PARS(double r=0,double d=0, double p=0, double rv=0) :
            ra(r), dec(d), parallax(p), radialVel(rv) {};

        /*
         * Assignment, copying etc quite ok.
         */
    };

    /*
     * An enum which is used to indicate the top-end in use and hence
     * distortion and focal ration.  These
     * are what we support at the moment. 
     *
     * The koala items are due to the different optics, and there
     * are two different field of views.  
     */
    enum TopEnd { F_8, F_15, PF_TRIPLET, F_8_KOALA_1, F_8_KOALA_2 };


    class telMdl {
    public:
        // Initialise the model for conversion at a given apparent field centre.
        // Note that RA and Dec are the field centre apparent RA and DEC as
        // radians.  Temperature here is in kelvin's   Wavelength is
        // in micrometers.
        //
        telMdl(const double mjd,  
               const double temp, 
               const double pres,  
               const double humid, 
               const TopEnd topEnd,
               const double cenWave, const double obsWave, 
               const double offset_x,             // Instrument offset 
               const double offset_y,
               const double instRotation,         // Instrument rotation
               const double pa,                   // PA value.
               const double ra, const double dec,
               const double distCentreX,
               const double distCentreY,
               StatusType *status);


        // We use the aao_cpp_util::field_property_get template to define
        // readonly (from an external point of view) properties.
        // Define the dateMjd property

        // MJD for observation.
        aao_cpp_util::field_property_get<double, double, telMdl> DateMjd; 
        // Field center RA (Apparent, radians).
        aao_cpp_util::field_property_get<double, double, telMdl> CenterRA;
        // Field center Dec (Apparent, radians).
        aao_cpp_util::field_property_get<double, double, telMdl> CenterDec;


        /* Convert from focal plane x/y to apparent place (approximate) */
        void Xy2App (
            double x,
            double y,
            double *ra,
            double *dec,
            StatusType *status) const;

        /* convert from apparent position to focal plane x/y */
        void App2Xy (
            double ra,
            double dec,
            double *x,
            double *y,
            StatusType *status) const;


        // Convert from positioner to Apparent RA/Dec. (precise, but slower)
        void Xy2AppIter (
            const double	x,
            const double	y,
            double		*ra,
            double		*dec,
            StatusType      *status,
            const double	tolerance = 10.0,
            const int	maxIters = 10) const;


        // Quick conversion from apparent to observed
        void QuickAppToObs(
            double ra,
            double dec,
            double *aob,
            double *zob,
            double *hob,
            double *dob,
            double *rob) const;


        // Convert from apparent RA/Dec to Tangent plane coordinates
        void App2Tan(
            double ra,
            double dec,
            double *xi,
            double *eta,
            StatusType *status) const;


        /* convert delta microns to arc-seconds using model */
        double Microns2Arcsec(
            double microns,
            StatusType *status) const;


        /* Return a specified telescope parameter - for inclusion in header */
        double GetTelPar(unsigned int num) const;

        // Destructor
        ~telMdl() {
        }

        /*
         * Enable/disable logging (currently only on initialization of model)
         *  Old setting is returned.
         */
        static bool SetLogging(bool enabled = true) {
            bool oldValue = _logInit;
            _logInit = enabled;
            return oldValue;
        }
        /*
         * Enable/disable the refraction calculations 
         *  Old setting is returned.
         */
        static bool SetDoRefraction(bool enabled = true) {
            bool oldValue = !_noRefraction;
            _noRefraction = !enabled;
            return oldValue;
        }
        /* 
         * Change the air-mass limit. Old value is returned 
         *
         *  The default value (10.0) of this will disable the checking
         *  of air-mass.  This check should be enabled by setting a
         *  reasonable value here (say 2) for code being used to say generate
         *  field configurations.
         */
        static double SetAirMassLimit(double newLimit) {
            double oldLimit = _airMassLimit;
            _airMassLimit = newLimit;
            return oldLimit;
        }

    protected:
        //  This allows the telMdlMean function to update the
        // field center after the telMdl contructor has been invoked.
        void SetAppPos(double ra, double dec) {
            CenterRA.m_value = ra;
            CenterDec.m_value = dec;
            double aob, hob, dob, rob;
            // Update Zenith Distance
            QuickAppToObs(ra, dec, &aob, &_zenithDistance, &hob, &dob, &rob);
        }

    private:
        /*
         * Parameters related to the linear transformation (actually, all
         * model parameters - type is misnamed)
         */
        struct {
            double coeffs[6];      /* Forward direction coefficents */
            double invCoeffs[6];   /* Reverse direction  coefficents */
            double rotation;       /* A rotation in degrees  */
            double offset_x;       /* Instrument offset in x */
            double offset_y;       /* Instrument offset in y */
            double focalLength;     /* Telescope focal length */
            double distPar1;       /* Telescope distortion parameter 1 */
            double distPar2;       /* Telescope distortion parameter 2 */
            double distPar3;       /* Telescope distortion parameter 3 */
            double distPar4;       /* Telescope distortion parameter 4 */

            double distCentreOffX; /* Centre of distortion in X (microns) */
            double distCentreOffY; /* Centre of distortion in Y (microns) */
        } _linPars;

        /*
         * Apparent to Observered parameters used by slaLib. 
         */
        static const int _numAoPars = 14;

        static bool _noRefraction;  /* If true, disable the refraction calculation */
        static bool _logInit;       /* If true, output logging of initialization */
        static double _airMassLimit;/* Complain if air-mass is greater then this */

        double _cenAoprms[_numAoPars];/* Apparent to observed parameters - field center wavelength */
        double _obsAoprms[_numAoPars];/* Apparent to observed parameters - observation wavelength  */
        double _cenLambda;      /*  Observing wavelength  */
        double _obsLambda;      /*  Observing wavelength  */


        double _dut;            // UT0-UT1

        double _zenithDistance; // Zenith distance at cenRa/cenDec

        /* Assignment and Copy operators, made private to avoid misuse */
        telMdl& operator=(const telMdl&); /* Assignment */
        telMdl(const telMdl&); /* Copy */


        /* Conversion from focal plane X/& to tanget plane */
        void Xy2Tan(double x,
                    double y,
                    double *xi,
                    double *eta,
                    StatusType *status) const;


        /* Validate  parameters */
        void ValidatePars(double mjd, double temp, double pres,
                          double humid, double cenWave,
                          double obsWave, const double rotation,
                          double ra, double dec, StatusType *status) const;

        /* Check if the specified ra/dec is observable at the specified MJD */
        void checkObservable (
            double mjd,
            double ra,
            double dec,
            StatusType *status) const;
        

        /* Log CvtInit details */
        void LogCvtInit(double mjd, double temp, double pres,
                        double humid, double cenWave,
                        double obsWave, const double rotation, double pa,
                        double ra, double dec) const;

        /* Validate App2Tan parameters */
        void ValidateApp2Tan(double ra,
                             double dec,
                             StatusType *status) const;


        /* convert from tanget plane position to focal plan x/y */
        void Tan2Xy  (
            const double xi,
            const double eta,
            double *x,
            double *y,
            StatusType *status) const;

        /* Apply the linear model part of the conversion */
        void ApplyLinearModel (
            double x,
            double y,
            double *xp,
            double *yp,
            StatusType *status) const;

        /* Normalze rotation, into a linear model */
        static void RotateLinear  (
            double rotation,
            double *a,
            double *b,
            double *c,
            double *d,
            double *e,
            double *f);



        /* Remove the linear model */
        void RemoveLinearModel (
            double xp,
            double yp,
            double *x,
            double *y,
            StatusType *status) const;

        /* Determine the focal length and pin cushion distortion effect parameter */
        void GetFocalLength(
            double *focalLength,
            double *pincushionCoeff) const;


        double applyCorrectorDistortion (const double r) const;

        /* Debugging function */
        void Dump() const;

        /* Used by contructor to help set up the class */
        void CreateModel (
            TopEnd topEnd,
            double rotation,
            double ypa,
            double offset_x,
            double offset_y,
            double distCentreX,
            double distCentreY,
            StatusType *status);




    };

    /*
     * This class sub-classes telMdl to provide an interface where
     * sky positions are mean rather then apparent.
     */
    class telMdlMean : public telMdl {
    public:
        static const int J2000 = 2000; // Standard equinox.

        // Some read-only properties. 

        // Field center RA (Mean, radians).
        aao_cpp_util::field_property_get<double, double, telMdlMean> CenterMeanRA;
        // Field center Dec (Mean, radians).
        aao_cpp_util::field_property_get<double, double, telMdlMean> CenterMeanDec;
        // Equinox of mean position (Mjd)
        aao_cpp_util::field_property_get<double, double, telMdlMean> Equinox;


        telMdlMean(const double mjd,  
                   const double temp, 
                   const double pres, 
                   const double humid, 
                   const TopEnd topEnd,
                   const double cenWave, 
                   const double obsWave, 
                   const double offset_x,             // Instrument offset 
                   const double offset_y,
                   const double instRotation,         // Instrument rotation
                   const double pa,                   // PA value.
                   const double ra, const double dec,
                   const double distCentreX,
                   const double distCentreY,
                   StatusType *status, double equinox=J2000);


        // Convert from Mean RA/DEC to focal plane.  Also returned the apparent
        // position (which is sometimes needed and we have already calculated
        // it.
        void Mean2Xy(
            double meanRa,
            double meanDec,
            const telMdlPM_PARS &,
            double *robotX,
            double *robotY,
            StatusType *status,
            double *appR = 0,
            double *appDec = 0) const;

        // Convert focal plane x/y coords to mean RA/Dec. (approximation)
        void Xy2Mean (
            const double	x,
            const double	y,
            double		*meanRA,
            double		*meanDec,
            StatusType          *status,
            const double	equinox = J2000) const;
        
        // Convert positioner coords to mean RA/Dec. (precise but slower)
        void Xy2MeanIter (
            const double	x,
            const double	y,
            double			*meanRA,
            double			*meanDec,
            StatusType      *status,
            const double	tolerance = 10.0,
            const int		maxIters = 10,
            const double	equinox = J2000) const;


        void App2Mean(
            const double appRA,
            const double appDec,
            double * meanRA,
            double * meanDec,
            StatusType *status,
            const double equinox = J2000) const ;

    private:
        double _gmap[21];       // sla library mean to apparent parameters
        //   see slaMappa for details.



        /* Assignment and Copy operators, made private to avoid misuse */
        telMdlMean& operator=(const telMdlMean&); /* Assignment */
        telMdlMean(const telMdlMean&); /* Copy */

    };

    /*
     * Structure used to return the results form the getProbeOffsets()
     * function below.
     */
    struct probeOffsets {
        double probe1ra;
        double probe1dec;
        double probe2ra;
        double probe2dec;

        probeOffsets() :
            probe1ra(0), probe1dec(0), probe2ra(0), probe2dec(0) {
        }
    };
    /*
     * Reset the offsets from the field center to two probes.
     */
    void GetProbeOffsets(
        bool verbose,
        double meanRA, double meanDec, const telMdlPM_PARS &pm, double equinox,
        double mjd, double temp, double press, double humid,
        TopEnd topEnd, double cenWave, double obsWave, 
        double x1, double y1, double x2, double y2, 
        probeOffsets *offsets,
        StatusType *status);

    /*
     * Put an offset into the range   -2*PI < offset <  2*PI
     */
    void NormalizeOffset(double *offset);
        

} // namespace AAT
#endif /*_CASS_TELMDL_H*/
