//
//                                A r r a y  M a n a g e r . c p p
//
//  Function:
//     Provides efficient and convenient C++ access to dynamic multi-dimensional arrays.
//
//  Description:
//     See the .h file for a detailed description of ArrayManager from a user's perspective.
//     This file provides the implementation. To follow the code, look at Malloc1D() first,
//     and then look at the code for the higher dimensional versions. Each adds one dimension
//     in turn, and it's easiest to look at them in order. it might have been possible to 
//     write one more complex routine that did the work of them all, but I think this way is
//     clearer, and probably infinitessimally more efficient.
//
//  Author: Keith Shortridge, AAO.
//
//  History:
//      1st Sep 2011. Original version. KS.
//      2nd Sep 2011. Free() now releases memory properly. KS.
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

#include "ArrayManager.h"

using std::list;

//  You can't do pointer arithmetic on void pointers, so the code has to use a byte type. I only
//  define Byte here because 'unsigned char' is too long to have all over the code.

typedef unsigned char Byte;

//  ------------------------------------------------------------------------------------------------

//                                      C o n s t r u c t o r
//
//  This is the only constructor.

ArrayManager::ArrayManager (void)
{
   I_Details.clear();
}

//  ------------------------------------------------------------------------------------------------

//                                      D e s t r u c t o r
//
//  The destructor releases all allocated arrays, including the various intermediate pointer
//  arrays used to simplify addressing.

ArrayManager::~ArrayManager()
{
   //  Iterate through all the arrays for which we have detais and clear them out.
   
   for (list<ArrayDetails>::iterator Iter = I_Details.begin(); Iter != I_Details.end(); Iter++) {
      for (int IDim = 0; IDim < Iter->NDims; IDim++) {
         if (Iter->Addresses[IDim]) {
            free(Iter->Addresses[IDim]);
            Iter->Addresses[IDim] = NULL;
         }
      }
   }
   I_Details.clear();
}

//  ------------------------------------------------------------------------------------------------

//                                           F r e e 
//
//   Free() releases an array that was allocated using one of the Malloc<n>D routines. It should
//   be passed the address that was returned by such a routine.

void ArrayManager::Free (
   void* Address)
{
   //  The address will be the address not of the main array, but of the highest dimensioned
   //  pointer array allocated for it. Look through the details list to find such an address,
   //  and if we find it release all the arrays used and erase the details.
   
   for (list<ArrayDetails>::iterator Iter = I_Details.begin(); Iter != I_Details.end(); Iter++) {
      if (Iter->Addresses[Iter->NDims - 1] == Address) {
         for (int IDim = 0; IDim < Iter->NDims; IDim++) {
            if (Iter->Addresses[IDim]) {
               free(Iter->Addresses[IDim]);
               Iter->Addresses[IDim] = NULL;
            }
         }
         I_Details.erase(Iter);
         break;
      }
   }
}

//  ------------------------------------------------------------------------------------------------

//                                      B a s e  A r r a y
//
//   BaseArray() returns the address of the actual element data for an array that was allocated 
//   using one of the Malloc<n>D routines. It should be passed the address that was returned by 
//   such a routine. The base address may be needed to load up the array by reading from a file,
//   or to write the data out, or some other purpose. There is no problem with going behind the
//   back of the ArrayManager code to do such things. (Although, obviously, attempting to access
//   memory outside that allocated for the array is almost certain to cause problems.)

void* ArrayManager::BaseArray (void* Address)
{
   //  The address will be the address not of the main array, but of the highest dimensioned
   //  pointer array allocated for it. Look through the details list to find such an address,
   //  and if we find it the address of the base array will be held in Addresses[0] for that
   //  array.
   
   void* BaseAddress = NULL;
   for (list<ArrayDetails>::iterator Iter = I_Details.begin(); Iter != I_Details.end(); Iter++) {
      if (Iter->Addresses[Iter->NDims - 1] == Address) {
         BaseAddress = Iter->Addresses[0];
         break;
      }
   }
   return BaseAddress;
}

//  ------------------------------------------------------------------------------------------------

//                               G e t  D i m e n s i o n s
//
//   GetDimensions() returns the dimensions of an array that was allocated using one of the 
//   Malloc<n>D routines. It should be passed the address that was returned by such a routine,
//   together with the address of an int to receive the number of dimensions of the array and
//   the address of a long array to receive the actual dimensions. It also needs to know the
//   size of that long array to ensure it doesn't exceed its bounds. If it cannot find any
//   reference to the array (it has to be one allocated by the same ArrayManager object as the
//   one currently invoked) it returns everything as zero. If the dimensions of the array are
//   fewer than NDims, higher elements of the Dims[] array are set to 1, so you multiply them
//   all up if necessary and get the right number of elements.

void ArrayManager::GetDimensions (
   void* Address,
   int MaxDims,
   int* NDims,
   long Dims[])
{   
   bool Found = false;
   
   //  The address will be the address not of the main array, but of the highest dimensioned
   //  pointer array allocated for it. Look through the details list to find such an address,
   //  and if we find it copy the dimension details back to the caller.
   
   for (list<ArrayDetails>::iterator Iter = I_Details.begin(); Iter != I_Details.end(); Iter++) {
      if (Iter->Addresses[Iter->NDims - 1] == Address) {
         *NDims = Iter->NDims;
         int ReportDims = Iter->NDims;
         if (ReportDims > MaxDims) ReportDims = MaxDims;
         for (int IDim = 0; IDim < ReportDims; IDim++) {
            Dims[IDim] = Iter->Dims[IDim];
         }
         for (int IDim = ReportDims; IDim < MaxDims; IDim++) {
            Dims[IDim] = 1;
         }
         Found = true;
         break;
      }
   }
   
   //  If we didn't find it, return a set of zeros.
   
   if (!Found) {
      for (int IDim = 0; IDim < MaxDims; IDim++) {
         Dims[IDim] = 0;
      }
      *NDims = 0;
   }
}

//  ------------------------------------------------------------------------------------------------

//                                           L i s t
//
//   List() outputs a list of the arrays currently allocated. If a null argument (the default)
//   is passed to it, the list is written to standard output. If a non-null argument is passed
//   then this should be the address of a routine that takes a single nul-terminated character
//   string and outputs it as a single line, and this routine will be called once for each
//   allocated array.

void ArrayManager::List (void (*ListRoutine)(const char* String))
{
   char DebugString[256];
   for (list<ArrayDetails>::iterator Iter = I_Details.begin(); Iter != I_Details.end(); Iter++) {
      long Elements = Iter->Dims[0];
      for (int Index = 1; Index < Iter->NDims; Index++) {
         Elements *= Iter->Dims[Index];
      }
      long Bytes = Elements * Iter->BytesPerElement;
      snprintf (DebugString,sizeof(DebugString),"%d-D array of %ld bytes at %p",
                                                Iter->NDims,Bytes,Iter->Addresses[0]);
      if (ListRoutine) {
         (*ListRoutine)(DebugString);
      } else {
         printf ("%s\n",DebugString);
      }
   }
}

//  ------------------------------------------------------------------------------------------------

//                                      M a l l o c  1 D
//
//  Malloc1D() allocates a 1-dimensional array of elements of the specified size. It is passed
//  the size of each element (usually a sizeof() call) and the number of elements. This is a 
//  simple operation and hardly taxes the code - all we do is return the address we get from 
//  malloc() - but it serves as a template for the higher-dimensional array allocation routines.

void* ArrayManager::Malloc1D (
   unsigned int BytesPerElement,
   long Nx)
{
   //  Allocate the array and get its address.
   
   Byte* Address = (Byte*) malloc (BytesPerElement * Nx);
   if (Address) {
     
      //  If it allocated OK, record the details in a structure and push a copy onto the
      //  I_Details list.
      
      ArrayDetails Details;
      for (int IDim = 0; IDim < 4; IDim++) {
         Details.Dims[IDim] = 0;
         Details.Addresses[IDim] = NULL;
      }
      Details.NDims = 1;
      Details.Dims[0] = Nx;
      Details.BytesPerElement = BytesPerElement;
      Details.Addresses[0] = Address;
      I_Details.push_back(Details);
   }
   return (void*) Address;
}

//  ------------------------------------------------------------------------------------------------

//                                      M a l l o c  2 D
//
//  Malloc2D() allocates a 2-dimensional array of elements of the specified size. It is passed
//  the size of each element (usually a sizeof() call) and the number of elements. Note that
//  we think of a 2D array as having Nx columns, and Ny rows. The data is stored so that the
//  data for each row is contiguous. We allocate an array for the actual data, and then
//  also allocate an array of pointers to the start of each row. This allows us to treat the
//  address of that array of pointers syntactically as a 2D array, in the sense that we can
//  refer to elements of the 2D array using Data[Iy][Ix].

void* ArrayManager::Malloc2D (
   unsigned int BytesPerElement,
   long Ny,
   long Nx)
{
   //  Allocate the data for the array (Nx by Ny bytes) and also allocate an array of Ny
   //  pointers to the start of each row.
   
   Byte* Address = (Byte*) malloc (BytesPerElement * Nx * Ny);
   Byte** RowAddresses = (Byte**) malloc (sizeof(Byte*) * Ny);
   if (Address && RowAddresses) {
   
      //  If we were able to allocate those OK, set the details into an array details structure
      //  and add a copy to the I_Details list of allocated arrays. This is like Malloc1D(),
      //  except that we need to set Dims[1] as well as Dims[0], but we also need to initialise
      //  the pointers in the RowAddresses array. And we save both addresses in the details
      //  structure.
      
      ArrayDetails Details;
      for (int IDim = 0; IDim < 4; IDim++) {
         Details.Dims[IDim] = 0;
         Details.Addresses[IDim] = NULL;
      }
      Details.NDims = 2;
      Details.Dims[0] = Nx;
      Details.Dims[1] = Ny;
      Details.BytesPerElement = BytesPerElement;
      Details.Addresses[0] = (void*) Address;
      Details.Addresses[1] = (void*) RowAddresses;
      I_Details.push_back(Details);
      
      //  The values for the RowAddresses are the addresses at which we find the start of
      //  the data for each row in the array starting at Address. Each row will be
      //  Nx * BytesPerElement bytes long.
      
      for (int Row = 0; Row < Ny; Row++) {
         RowAddresses[Row] = Address + (Row * Nx * BytesPerElement);
      }
   } else {
   
      //  If either malloc call failed, make sure we release the other. Releasing both
      //  like this works.
      
      if (Address) free(Address);
      if (RowAddresses) free(RowAddresses);
      Address = NULL;
      RowAddresses = NULL;
   }
   return (void*) RowAddresses;
}

//  ------------------------------------------------------------------------------------------------

//                                      M a l l o c  3 D
//
//  Malloc3D() allocates a 3-dimensional array of elements of the specified size. It is passed
//  the size of each element (usually a sizeof() call) and the number of elements. Note that
//  we think of a 3D array as having Nx columns, Ny rows, and Nz planes. The data is stored so
//  that the data for each XY plane is contiguous, and that for each row in X within that plane
//  is also contiguous. We allocate an array for the actual data, and then also allocate an array
//  of pointers to the start of each row and then another array of pointers to the start of
//  the row pointers for each plane. This allows us to treat the address of that final array of 
//  pointers syntactically as a 3D array, in the sense that we can refer to elements of the 3D 
//  array using Data[Iz][Iy][Ix].

void* ArrayManager::Malloc3D (
   unsigned int BytesPerElement,
   long Nz,
   long Ny,
   long Nx)
{
   //  If you've looked at the code for Malloc2D(), all this does is add one more dimension.
   
   Byte* Address = (Byte*) malloc (BytesPerElement * Nx * Ny * Nz);
   Byte** RowAddresses = (Byte**) malloc (sizeof(Byte*) * Ny * Nz);
   Byte*** PlaneAddresses = (Byte***) malloc (sizeof(Byte**) * Nz);
   if (Address && RowAddresses && PlaneAddresses) {
      ArrayDetails Details;
      for (int IDim = 0; IDim < 4; IDim++) {
         Details.Dims[IDim] = 0;
         Details.Addresses[IDim] = NULL;
      }
      Details.NDims = 3;
      Details.Dims[0] = Nx;
      Details.Dims[1] = Ny;
      Details.Dims[2] = Nz;
      for (int Row = 0; Row < (Ny * Nz); Row++) {
         RowAddresses[Row] = Address + (Row * Nx * BytesPerElement);
      }
      for (int Plane = 0; Plane < Nz; Plane++) {
         PlaneAddresses[Plane] = RowAddresses + (Plane * Ny);
      }
      Details.BytesPerElement = BytesPerElement;
      Details.Addresses[0] = (void*) Address;
      Details.Addresses[1] = (void*) RowAddresses;
      Details.Addresses[2] = (void*) PlaneAddresses;
      I_Details.push_back(Details);
   } else {
      if (Address) free(Address);
      if (RowAddresses) free(RowAddresses);
      if (PlaneAddresses) free(PlaneAddresses);
      Address = NULL;
      RowAddresses = NULL;
      PlaneAddresses = NULL;
   }
   return (void*) PlaneAddresses;
}

//  ------------------------------------------------------------------------------------------------

//                                      M a l l o c  4 D
//
//  Malloc4D() allocates a 4-dimensional array of elements of the specified size. It is passed
//  the size of each element (usually a sizeof() call) and the number of elements. Note that
//  we think of a 4D array as having Nx columns, Ny rows, Nz planes, and Nt cubes. The data is 
//  stored so that the data for each XYZ cube is contiguous, and the data for each XY plane within
//  that cube is contiguous, and so is that for each row in X within that plane. We allocate an 
//  array for the actual data, and then also allocate an array of pointers to the start of each 
//  row and then another array of pointers to the start of the row pointers for each plane (so far
//  just like Malloc3D, except that there are more elements to the data) and finally yet another
//  array of pointers to the start of the plane pointers for each cube. This allows us to treat 
//  the address of that final array of pointers syntactically as a 4D array, in the sense that we 
//  can refer to elements of the 4D array using Data[It][Iz][Iy][Ix].

void* ArrayManager::Malloc4D (
   unsigned int BytesPerElement,
   long Nt,
   long Nz,
   long Ny,
   long Nx)
{
   //  If you've looked at the code for Malloc3D(), all this does is add one more dimension,
   //  although I admit that by the time you have four asterisks in a row it starts to get
   //  scary.
   
   Byte* Address = (Byte*) malloc (BytesPerElement * Nx * Ny * Nz * Nt);
   Byte** RowAddresses = (Byte**) malloc (sizeof(Byte*) * Ny * Nz * Nt);
   Byte*** PlaneAddresses = (Byte***) malloc (sizeof(Byte**) * Nz * Nt);
   Byte**** CubeAddresses = (Byte****) malloc (sizeof(Byte***) * Nt);
   if (Address && RowAddresses && PlaneAddresses && CubeAddresses) {
      ArrayDetails Details;
      for (int IDim = 0; IDim < 4; IDim++) {
         Details.Dims[IDim] = 0;
         Details.Addresses[IDim] = NULL;
      }
      Details.NDims = 4;
      Details.Dims[0] = Nx;
      Details.Dims[1] = Ny;
      Details.Dims[2] = Nz;
      Details.Dims[3] = Nt;
      for (int Row = 0; Row < (Ny * Nz * Nt); Row++) {
         RowAddresses[Row] = Address + (Row * Nx * BytesPerElement);
      }
      for (int Plane = 0; Plane < Nz * Nt; Plane++) {
         PlaneAddresses[Plane] = RowAddresses + (Plane * Ny);
      }
      for (int Cube = 0; Cube < Nt; Cube++) {
         CubeAddresses[Cube] = PlaneAddresses + (Cube * Nz);
      }
      Details.BytesPerElement = BytesPerElement;
      Details.Addresses[0] = (void*) Address;
      Details.Addresses[1] = (void*) RowAddresses;
      Details.Addresses[2] = (void*) PlaneAddresses;
      Details.Addresses[3] = (void*) CubeAddresses;
      I_Details.push_back(Details);
   } else {
      if (Address) free(Address);
      if (RowAddresses) free(RowAddresses);
      if (PlaneAddresses) free(PlaneAddresses);
      if (CubeAddresses) free(CubeAddresses);
      Address = NULL;
      RowAddresses = NULL;
      PlaneAddresses = NULL;
      CubeAddresses = NULL;
   }
   return (void*) CubeAddresses;
}

//  ------------------------------------------------------------------------------------------------

//                                     T e s t  C o d e
//
//  This is a pretty basic test routine that at least exercises most of the facilities
//  of the ArrayManager code. It creates arrays of all the supported dimensionalities and
//  collapses the large down into the smaller, successively, until all the data in the
//  largest array has been collapsed down into just one number. This is compared to the
//  value obtained by summing the elements of the original raw data in the largest
//  array and seeing if they match.
//
//  Building:
//     g++ -Wall -ansi -pedantic -o Test -DTEST_ACCESS ArrayManager.cpp
//     ./Test

#ifdef TEST_ACCESS

int main ()
{
   int Nt = 2;
   int Nz = 3;
   int Ny = 4;
   int Nx = 5;
   ArrayManager Manager;
   
   //  Allocate a 4D array and fill it up with test values.
   
   int**** Int4dArray = (int****) Manager.Malloc4D (sizeof(int),Nt,Nz,Ny,Nx);
   if (!Int4dArray) {
      printf ("***Failed to allocate 4D array***\n");
   } else {
      for (int It = 0; It < Nt; It++) {
         for (int Iz = 0; Iz < Nz; Iz++) {
            for (int Iy = 0; Iy < Ny; Iy++) {
               for (int Ix = 0; Ix < Nx; Ix++) {
                  Int4dArray[It][Iz][Iy][Ix] = It + Iz + Iy + Ix;
               }
            }
         }
      }
      
      //  Allocate a 3D array and collapse the 4D array down into it.
      
      float*** Float3dArray = (float***) Manager.Malloc3D (sizeof(float),Nz,Ny,Nx);
      if (!Float3dArray) {
         printf ("***Failed to allocate 3D array***\n");
      } else {
         for (int Iz = 0; Iz < Nz; Iz++) {
            for (int Iy = 0; Iy < Ny; Iy++) {
               for (int Ix = 0; Ix < Nx; Ix++) {
                  float Total = 0.0;
                  for (int It = 0; It < Nt; It++) {
                     Total += Int4dArray[It][Iz][Iy][Ix];
                  }
                  Float3dArray[Iz][Iy][Ix] = Total;
               }
            }
         }
         
         //  Allocate a 2D array and collapse the 3D array into it.
         
         int** Int2dArray = (int**) Manager.Malloc2D (sizeof(int),Ny,Nx);
         if (!Int2dArray) {
            printf ("***Failed to allocate 2D array***\n");
         } else {
            for (int Iy = 0; Iy < Ny; Iy++) {
               for (int Ix = 0; Ix < Nx; Ix++) {
                  int Total = 0;
                  for (int Iz = 0; Iz < Nz; Iz++) {
                     Total += Float3dArray[Iz][Iy][Ix];
                  }
                  Int2dArray[Iy][Ix] = Total;
               }
            }
            
            //  Allocate a 1D array and collapse the 2D array into it.
            
            float* Float1dArray = (float*) Manager.Malloc1D (sizeof(int),Nx);
            if (!Float1dArray) {
               printf ("***Failed to allocate 1D array***\n");
            } else {
               for (int Ix = 0; Ix < Nx; Ix++) {
                  float Total = 0.0;
                  for (int Iy = 0; Iy < Ny; Iy++) {
                     Total += Int2dArray[Iy][Ix];
                  }
                  Float1dArray[Ix] = Total;
               }
               
               //  Now add up all the elements of the 1D array.
               
               float Total = 0.0;
               for (int Ix = 0; Ix < Nx; Ix++) {
                  Total += Float1dArray[Ix];
               }
               
               //  And we can also get the base data array of the original 4D array -
               //  that is its data as a linear array as allocated - and sum it as
               //  well. That should match the Total we already have.
               
               int* Raw4dIntArray = (int*) Manager.BaseArray (Int4dArray);
               if (!Raw4dIntArray) {
                  printf ("***Failed to get address of raw 4D array***\n");
               } else {
                  long Dims[4];
                  int NDims;
                  Manager.GetDimensions (Int4dArray,4,&NDims,Dims);
                  if (NDims == 0) {
                     printf ("***Failed to get dimensions of 4D array***\n");
                  } else {
                     int Nelm = 1;
                     for (int IDim = 0; IDim < NDims; IDim++) {
                        Nelm *= Dims[IDim];
                     }
                     if (Nelm != Nt * Nz * Ny * Nx) {
                        printf ("***Dimensions from GetDimensions don't match***\n");
                     } else {
                        float RawTotal = 0.0;
                        for (int I = 0; I < Nelm; I++) {
                           RawTotal += Raw4dIntArray[I];
                        }
                        if (RawTotal != Total) {
                           printf ("***Total mismatch***\n");
                        } else {
                           printf ("Final total (%f) is OK\n",Total);
                        }
                     }
                  }
               }
            }
         }
      }
   }
   Manager.List();
   return 0;
}
                      
#endif
                     
         
      
      
