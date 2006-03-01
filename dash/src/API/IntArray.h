#ifndef DAQHWYAPI_INTARRAY_H
#define DAQHWYAPI_INTARRAY_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_MAINDEFS_H
#include <dshapi/maindefs.h>
#endif

namespace daqhwyapi {

/**
* @class IntArray
* @brief IntArray class.
*
* The IntArray class for storing integers as an Array. 
*
* @author  Eric Kasten
* @version 1.0.0
*/
class IntArray : public Object {
  public: 
    /**
    * @var length
    * @brief Length of this array.
    *
    * Length of the array as a count of ints.
    */
    unsigned long length;

    /**
    * @var elements
    * @brief Integer elements stored in this IntArray.
    *
    * The array of integer elements stored in this IntArray.
    */
    int* elements;
 
    IntArray();           // Default constructor 
    IntArray(const IntArray&);  // Copy constructor 
    IntArray(unsigned long);  // Constructor with size 
    IntArray(const int*,unsigned long);  // Constructor with elements
    virtual ~IntArray();          // Destructor
    IntArray& operator=(const IntArray&);  // Assignment  
    void clear();  // Empty this Array.
    void renew(unsigned long);  // Renew this array

  protected:
    void copyToThis(const IntArray&);
};

} // namespace daqhwyapi

#endif
