#ifndef DAQ_CHARARRAY_H
#define DAQ_CHARARRAY_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#ifndef DAQCONFIG_H
#include <daqconfig.h>
#endif

#ifndef DAQOBJECT_H
#include <DAQObject.h>
#endif

/**
* @class DAQCharArray
* @brief DAQCharArray class.
*
* The DAQCharArray class for storing integers as an Array. 
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DAQCharArray : public DAQObject {
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
    * @brief Pointer for elements stored in this array.
    *
    * The array of characters stored in this array.
    */
    char *elements;

    DAQCharArray();           // Default constructor 
    DAQCharArray(const DAQCharArray&);   // Copy constructor 
    DAQCharArray(unsigned long);   // Constructor with size
    DAQCharArray(int*,unsigned long);  // Constructor with elements
    virtual ~DAQCharArray();          // Destructor
    DAQCharArray& operator=(const DAQCharArray&);  // Assignment
    void clear(); // Empty this array 
    void renew(unsigned long);       // Renew this array
    void resize(unsigned long);  // Rsize this array

  protected:
    void copyToThis(const DAQCharArray&);
};

#endif
