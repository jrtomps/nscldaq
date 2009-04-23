#ifndef DAQ_OBJECTARRAY_H
#define DAQ_OBJECTARRAY_H

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
* @class DAQObjectArray
* @brief DAQObjectArray class.
*
* The DAQObjectArray class for storing DAQObjects as an Array. 
*
* @author  Eric Kasten
* @version 1.0.0
*/
class DAQObjectArray : public DAQObject {
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
    * @brief Object pointer for elements stored in this ObjectArray.
    *
    * The array of Object pointers stored in this ObjectArray.
    */
    DAQObject** elements;

    DAQObjectArray();           // Default constructor 
    DAQObjectArray(const DAQObjectArray&);   // Copy constructor 
    DAQObjectArray(unsigned long);   // Constructor with size
    DAQObjectArray(DAQObject**,unsigned long);  // Constructor with elements
    virtual ~DAQObjectArray();          // Destructor
    DAQObjectArray& operator=(const DAQObjectArray&);  // Assignment
    void clear(); // Empty this array 
    void clearAndDelete(); // Empty this array and delete the elements
    void renew(unsigned long);       // Renew this array
    void resize(unsigned long);  // Rsize this array

  protected:
    void copyToThis(const DAQObjectArray&);
};

#endif
