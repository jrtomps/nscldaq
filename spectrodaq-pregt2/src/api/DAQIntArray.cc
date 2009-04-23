/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#ifndef DAQOBJECT_H
#include <DAQObject.h>
#endif

#ifndef DAQ_INTARRAY_H
#include <DAQIntArray.h>
#endif

/*==============================================================*/
/** @fn DAQIntArray::DAQIntArray()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
DAQIntArray::DAQIntArray() {
  elements = NULL;
  length = 0;
}

/*==============================================================*/
/** @fn DAQIntArray::DAQIntArray(const DAQIntArray& rArry)
* @brief Copy constructor.
*
* Copy constructor.
*
* @param rArry The other array.
* @return this
*/                                                             
DAQIntArray::DAQIntArray(const DAQIntArray& rArry) {
  elements = NULL;
  length = 0;
  copyToThis(rArry);
}

/*==============================================================*/
/** @fn DAQIntArray::DAQIntArray(unsigned long l)
* @brief Constructor with a size.
*
* Constructor with a size.
*
* @param l The length of the array.
* @return this
*/                                                             
DAQIntArray::DAQIntArray(unsigned long l) {
  elements = NULL;
  length = l;
  elements = new int[length];
  for (int i = 0; i < length; i++) elements[i] = 0;
}

/*==============================================================*/
/** @fn DAQIntArray::DAQIntArray(int pElems[],unsigned long l)
* @brief Constructor with elements.
*
* Constructor with elements.
*
* @param pElems The elemets to use for construction.
* @param l The length of pElemes.
* @return this
*/                                                             
DAQIntArray::DAQIntArray(int pElems[],unsigned long l) {
  elements = NULL;
  length = l;
  elements = new int[length];
  if (pElems != NULL) {
    for (int i = 0; i < length; i++) elements[i] = pElems[i];
  } else {
    for (int i = 0; i < length; i++) elements[i] = 0;
  }
}

/*==============================================================*/
/** @fn DAQIntArray::~DAQIntArray() 
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
DAQIntArray::~DAQIntArray() {
  clear();
}

/*==============================================================*/
/** @fn void DAQIntArray::clear() 
* @brief Clear this array.
*
* Clear this array.
*
* @param None
* @return None
*/                                                             
void DAQIntArray::clear() {
  if (elements != NULL) {
    delete[] elements;
    elements = NULL;
  }
  length = 0; 
}

/*==============================================================*/
/** @fn DAQIntArray& DAQIntArray::operator=(const DAQIntArray& ary) 
* @brief Assign an array to this one.
*
* Assign another DAQIntArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
DAQIntArray& DAQIntArray::operator=(const DAQIntArray& ary) {
  copyToThis(ary);
  return(*this);
} 

/*==============================================================*/
/** @fn void DAQIntArray::copyToThis(const DAQIntArray& ary) 
* @brief Assign an array to this one.
*
* Assign another DAQIntArray to this one.
*
* @param ary The other array.
* @return None
*/                                                             
void DAQIntArray::copyToThis(const DAQIntArray& ary) {
  clear();
  length = ary.length;
  elements = new int[length];
  if (ary.elements != NULL) {
    for (int i = 0; i < length; i++) elements[i] = ary.elements[i];
  } else {
    for (int i = 0; i < length; i++) elements[i] = 0;
  }
} 

/*==============================================================*/
/** @fn void DAQIntArray::renew(unsigned long l) 
* @brief Empty this array and resize it.
*
* Empty this array and resize it.  Destroys existing 
* elements.
*
* @param l New size of this array.
* @return None
*/                                                             
void DAQIntArray::renew(unsigned long l) {
  if (l == length) {
    if (elements != NULL) {
      for (int i = 0; i < length; i++) elements[i] = 0;
    }
  } else {
    clear();
    length = l;
    elements = new int[length];
    for (int i = 0; i < length; i++) elements[i] = 0; 
  }
}

/*==============================================================*/
/** @fn void resize(unsigned long l)
* @brief Resize this array without loss of data.
*
* Resize this array without loss of data
*
* @param l New length.
* @return None
*/
void DAQIntArray::resize(unsigned long l) {
  if (l != length) {
    DAQIntArray old(*this);
    int lmin = (l < length) ? l : length;
    this->renew(l);
    for (int i = 0; i < lmin ; i++) {
      elements[i] = old.elements[i];
    }
  }
}

