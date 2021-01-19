//
//                               A r r a y  M a n a g e r . h
//
//  Function:
//     Provides efficient and convenient C++ access to dynamic multi-dimensional arrays.
//
//  Description:
//     This is a rather basic set of routines that allow C++ programs to allocate multi-
//     dimensional arrays and access them as if they had been allocated statically. For
//     example, a 3D array of floats can be allocated using:
//
//     ArrayManager Accessor;
//     float*** Data = (float***) Accessor.Malloc3D(sizeof(float),Nz,Ny,Nx);
//
//     and then accessed using:
//
//     float Element = Data[Iz][Iy][Ix];
//
//     There are routines to allocate 1D,2D,3D and 4D arrays of any type. It might be nice
//     to have this implemented using templates so the casts in the Malloc calls can be
//     avoided, but this is effective and simplifies things considerably.
//
//     This sort of functionality is also provided, perhaps more elegantly, by the Boost
//     multi-dimensional array library, or by Blitz++, but these are implemented using 
//     rather esoteric C++ template code and rely a lot on the efficiency of the compilers.
//     This is a much simpler piece of code (I wrote it in an afternoon) and it does all
//     that I feel I need at the time of writing. Note also that somewhere along the way
//     C++ acquired the ability to declare multi-dimensional arrays dynamically on the stack,
//     but this is probably not a solution for large arrays or arrays you may want to stick
//     around for a while.
//   
//     Each ArrayManager object can handle any number of array allocations. When it is
//     destroyed, all the arrays it has allocated are deleted. Arrays can be deleted
//     explicitly using ArrayManager::Free(), and ArrayManager::List() can be used to list
//     the arrays currently allocated by the ArrayManager object. 
//
//     Note that the array returned by ArrayManager::Malloc3D() and its kin is not the actual
//     array containing the individual floating point values. In the case of Malloc3D() it is
//     an array of Nz pointers each to an array of Ny pointers to the rows of the actual array.
//     These additional arrays are allocated by the ArrayManager object to allow you to access 
//     the data in the convenient (Fortran-like, perhaps) way shown above. If you do want the
//     address of the actual array of floating point values, which you might if you wanted
//     to write them out, or to read data into the array, you could always use
//     &(Data[0][0][0][0]) but that looks odd (at least to me) and so there is a separate
//     ArrayManager::BaseArray() call provided to give you that address directly.
//
//     Given the address returned by one of the Malloc() routines, you can pass this to
//     GetDimensions() to find out the dimensions of the array. This means you can pass the
//     address to a subroutine without needing to also pass the array dimensions, for example.
//
//     The idea of this code is to allow you to get elements of an N-dimensional array
//     directly without having to do the usual C thing of manipulating pointers yourself
//     in what I always find is a very error-prone process. It should be efficient. For
//     example, in code such as:
//
//     for (int Iz = 0; Iz < Nz; Iz++) {
//        for (int Iy = 0; Iy < Ny; Iy++) {
//           for (int Ix = 0; Ix < Nx; Ix++) {
//               do something with Data[Iz][Iy][Ix]
//           }
//        }
//     }
//
//     any decent compiler ought to be able to spot that Data[Iz] is invariant throughout
//     the two inner loops, and Data[Iz][Iy] is invariant through the inner Ix loop, and
//     should be able to generate reasonably efficient code. (I've not actually verified
//     this, by the way - I'm happy enough with the convenience.)
//
//  Author: Keith Shortridge, AAO.
//
//  History:
//      1st Sep 2011. Original version.
//     27th Apr 2012. Added include of stdlib.h. and stdio.h. KS.
//     11th Aug 2019. Added an output routine as an argument to List(). This means that
//                    this code no longer assumes there is an external routine called
//                    DEBUGLog(). However, calls to List() may have to be revised, as this
//                    change is not backwards-compatible(). Licence text is now the more
//                    permissive version currently used by AAO. KS.
//
//  Copyright (c) Australian Astronomical Observatory (AAO), 2019.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without
//  restriction, including without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <list>
#include <stdlib.h>
#include <stdio.h>

//  One struct of type ArrayDetails is used for each allocated array.

typedef struct {
   //! Array dimensions
   long Dims[4];
   //! Number of dimensions
   int NDims;
   //! Number of bytes in each array element
   int BytesPerElement;
   //! The addresses of the various allocated arrays. [0] is the address of the base 
   //! array. [1] is the address of the pointers to each row of the data, [2] is the 
   //! address of an array of pointers each pointing to the row pointer that starts a
   //! new plane of the data, [3] is the address of an array of pointers each pointing
   //! to the plane pointer that starts a new cube of the data. And so it could go on.. 
   void* Addresses[4];
} ArrayDetails;


class ArrayManager {
public:
   //!  Constructor
   ArrayManager (void);
   //!  Destructor
   ~ArrayManager ();
   //!  Allocate a 4-dimensional array of any type.
   void* Malloc4D (unsigned int BytesPerElement,long Nt,long Nz,long Ny,long Nx);
   //!  Allocate a 3-dimensional array of any type.
   void* Malloc3D (unsigned int BytesPerElement,long Nz,long Ny,long Nx);
   //!  Allocate a 2-dimensional array of any type.
   void* Malloc2D (unsigned int BytesPerElement,long Ny,long Nx);
   //!  Allocate a 1-dimensional array of any type.
   void* Malloc1D (unsigned int BytesPerElement,long Nx);
   //!  Release an array allocated by one of the Malloc() routines.
   void Free (void* Address);
   //!  Return the address of the actual elements of the array.
   void* BaseArray (void* Address);
   //!  Return the dimensions of the array.
   void GetDimensions (void* Address, int MaxDims, int* NDims, long Dims[]);
   //!  List the allocated arrays for diagnostic purposes.
   void List (void (*ListRoutine)(const char* String) = NULL);
private:
   //!  The set of descriptor structures for each array currently allocated.
   std::list<ArrayDetails> I_Details;
};
   
   
