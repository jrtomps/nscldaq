#ifndef DAQHWYAPI_BYTEARRAY_H
#define DAQHWYAPI_BYTEARRAY_H

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

#ifndef DAQHWYAPI_DSHARRAY_H
#include <dshapi/DSHArray.h>
#endif

#ifndef DAQHWYAPI_MAINDEFS_H
#include <dshapi/maindefs.h>
#endif

namespace daqhwyapi {

/**
* @class ByteArray
* @brief ByteArray class.
*
* The ByteArray class for storing unsigned characters (ubytes) as an Array. 
*
* @author  Eric Kasten
* @version 1.0.0
*/
class ByteArray : public DSHArray {
  public: 
    /**
    * @var length
    * @brief Length of this array.
    *
    * Length of the array as a count of bytes.
    */
    unsigned long length;

    /**
    * @var elements
    * @brief Byte elements stored in this IntArray.
    *
    * The array of ubyte elements stored in this ByteArray.
    */
    ubyte* elements;
 
    ByteArray();           // Default constructor 
    ByteArray(const ByteArray&);  // Copy constructor 
    ByteArray(unsigned long);  // Constructor with size 
    ByteArray(const ubyte*,unsigned long);  // Constructor with elements
    virtual ~ByteArray();          // Destructor
    ByteArray& operator=(const ByteArray&);  // Assignment  
    void clear();  // Empty this Array.
    void renew(unsigned long);  // Renew this array

  protected:
    void copyToThis(const ByteArray&);
};

} // namespace daqhwyapi

#endif
