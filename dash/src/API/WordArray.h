#ifndef DAQHWYAPI_WORDARRAY_H
#define DAQHWYAPI_WORDARRAY_H

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
* @class WordArray
* @brief WordArray class.
*
* The WordArray class for storing unsigned characters (uwords) as an Array. 
*
* @author  Eric Kasten
* @version 1.0.0
*/
class WordArray : public DSHArray {
  public: 
    /**
    * @var length
    * @brief Length of this array.
    *
    * Length of the array as a count of words.
    */
    unsigned long length;

    /**
    * @var elements
    * @brief Word elements stored in this WordArray.
    *
    * The array of uword elements stored in this WordArray.
    */
    uword* elements;
 
    WordArray();           // Default constructor 
    WordArray(const WordArray&);  // Copy constructor 
    WordArray(unsigned long);  // Constructor with size 
    WordArray(const uword*,unsigned long);  // Constructor with elements
    virtual ~WordArray();          // Destructor
    WordArray& operator=(const WordArray&);  // Assignment  
    void clear();  // Empty this Array.
    void renew(unsigned long);  // Renew this array

  protected:
    void copyToThis(const WordArray&);
};

} // namespace daqhwyapi

#endif
