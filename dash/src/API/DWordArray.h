#ifndef DAQHWYAPI_DWORDARRAY_H
#define DAQHWYAPI_DWORDARRAY_H

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
* @class DWordArray
* @brief DWordArray class.
*
* The DWordArray class for storing double words as an Array. 
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DWordArray : public DSHArray {
  public: 
    /**
    * @var length
    * @brief Length of this array.
    *
    * Length of the array as a count of double words.
    */
    unsigned long length;

    /**
    * @var elements
    * @brief Double word elements stored in this DWordArray.
    *
    * The array of duword elements stored in this DWordArray.
    */
    duword *elements;
 
    DWordArray();           // Default constructor 
    DWordArray(const DWordArray&);  // Copy constructor 
    DWordArray(unsigned long);  // Constructor with size 
    DWordArray(const duword*,unsigned long);  // Constructor with elements
    virtual ~DWordArray();          // Destructor
    DWordArray& operator=(const DWordArray&);  // Assignment  
    void clear();  // Empty this Array.
    void renew(unsigned long);  // Renew this array

  protected:
    void copyToThis(const DWordArray&);
};

} // namespace daqhwyapi

#endif
