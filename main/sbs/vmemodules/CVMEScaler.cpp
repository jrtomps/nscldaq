/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";   
//////////////////////////CVMEScaler.cpp file////////////////////////////////////
#include <config.h>
#include "CVMEScaler.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

                  

	//Default constructor alternative to compiler provided default constructor
	//Association object data member pointers initialized to null association object 
/*!
   Default constructor.  This is called when declarations of the form e.g.:
   -  CVMEScaler  object;
   are performed.
*/
CVMEScaler::CVMEScaler ()
 
{

} 
/*!
   Copy construction. This is invoked when e.g. an object is passed by value
   to a function.  The copy constructor makes a clone of the rhs object.
*/
CVMEScaler::CVMEScaler(const CVMEScaler& rhs) 
{

}

	//Operator= Assignment Operator alternative to compiler provided operator=  

/*!
   Assignment operation.  This member function supports assignment of
   an object of this class to an object of the same class.
*/
CVMEScaler& CVMEScaler::operator= (const CVMEScaler& aCVMEScaler)
{ 
    if (this != &aCVMEScaler) {
    }
    return *this;
}

/*!
   Comparison for equality.
   */
int
CVMEScaler::operator==(const CVMEScaler& rhs) const
{
  return 1;
}

