#ifndef DAQHWYAPI_STRINGARRAY_H
#define DAQHWYAPI_STRINGARRAY_H

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

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

namespace daqhwyapi {

/**
* @class StringArray
* @brief StringArray class.
*
* The StringArray class for storing Strings as an Array. 
*
* @author  Eric Kasten
* @version 1.0.0
*/
class StringArray : public DSHArray {
  public: 
    /**
    * @var length
    * @brief Length of this array.
    *
    * Length of the array as a count of characters.
    */
    unsigned long length;

    /**
    * @var elements
    * @brief String pointer elements stored in this StringArray.
    *
    * The array of String pointers stored in this StringArray.
    */
    String** elements;

    StringArray();           // Default constructor 
    StringArray(const StringArray&);   // Copy constructor 
    StringArray(unsigned long);   // Constructor with size
    StringArray(String**,unsigned long);  // Constructor with elements
    virtual ~StringArray();          // Destructor
    StringArray& operator=(const StringArray&);  // Assignment
    void clear(); // Empty this array 
    void clearAndDelete(); // Empty this array and delete the elements
    void renew(unsigned long);       // Renew this array

  protected:
    void copyToThis(const StringArray&);
};

} // namespace daqhwyapi

#endif
