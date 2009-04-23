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

#ifndef DAQ_CHARARRAY_H
#include <DAQCharArray.h>
#endif

/*==============================================================*/
/** @fn DAQCharArray::DAQCharArray()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
DAQCharArray::DAQCharArray() {
  elements = NULL;
  length = 0;
}

/*==============================================================*/
/** @fn DAQCharArray::DAQCharArray(const DAQCharArray& rArry)
* @brief Copy constructor.
*
* Copy constructor.
*
* @param rArry The other array.
* @return this
*/                                                             
DAQCharArray::DAQCharArray(const DAQCharArray& rArry) {
  elements = NULL;
  length = 0;
  copyToThis(rArry);
}

/*==============================================================*/
/** @fn DAQCharArray::DAQCharArray(unsigned long l)
* @brief Constructor with a size.
*
* Constructor with a size.
*
* @param l The length of the array.
* @return this
*/                                                             
DAQCharArray::DAQCharArray(unsigned long l) {
  elements = NULL;
  length = l;
  elements = new char[length];
  for (int i = 0; i < length; i++) elements[i] = ' ';
}

/*==============================================================*/
/** @fn DAQCharArray::DAQCharArray(int pElems[],unsigned long l)
* @brief Constructor with elements.
*
* Constructor with elements.
*
* @param pElems The elemets to use for construction.
* @param l The length of pElemes.
* @return this
*/                                                             
DAQCharArray::DAQCharArray(int pElems[],unsigned long l) {
  elements = NULL;
  length = l;
  elements = new char[length];
  if (pElems != NULL) {
    for (int i = 0; i < length; i++) elements[i] = pElems[i];
  } else {
    for (int i = 0; i < length; i++) elements[i] = ' ';
  }
}

/*==============================================================*/
/** @fn DAQCharArray::~DAQCharArray() 
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
DAQCharArray::~DAQCharArray() {
  clear();
}

/*==============================================================*/
/** @fn void DAQCharArray::clear() 
* @brief Clear this array.
*
* Clear this array.
*
* @param None
* @return None
*/                                                             
void DAQCharArray::clear() {
  if (elements != NULL) {
    delete[] elements;
    elements = NULL;
  }
  length = 0; 
}

/*==============================================================*/
/** @fn DAQCharArray& DAQCharArray::operator=(const DAQCharArray& ary) 
* @brief Assign an array to this one.
*
* Assign another DAQCharArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
DAQCharArray& DAQCharArray::operator=(const DAQCharArray& ary) {
  copyToThis(ary);
  return(*this);
} 

/*==============================================================*/
/** @fn void DAQCharArray::copyToThis(const DAQCharArray& ary) 
* @brief Assign an array to this one.
*
* Assign another DAQCharArray to this one.
*
* @param ary The other array.
* @return None
*/                                                             
void DAQCharArray::copyToThis(const DAQCharArray& ary) {
  clear();
  length = ary.length;
  elements = new char[length];
  if (ary.elements != NULL) {
    for (int i = 0; i < length; i++) elements[i] = ary.elements[i];
  } else {
    for (int i = 0; i < length; i++) elements[i] = ' ';
  }
} 

/*==============================================================*/
/** @fn void DAQCharArray::renew(unsigned long l) 
* @brief Empty this array and resize it.
*
* Empty this array and resize it.  Destroys existing 
* elements.
*
* @param l New size of this array.
* @return None
*/                                                             
void DAQCharArray::renew(unsigned long l) {
  if (l == length) {
    if (elements != NULL) {
      for (int i = 0; i < length; i++) elements[i] = ' ';
    }
  } else {
    clear();
    length = l;
    elements = new char[length];
    for (int i = 0; i < length; i++) elements[i] = ' '; 
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
void DAQCharArray::resize(unsigned long l) {
  if (l != length) {
    DAQCharArray old(*this);
    int lmin = (l < length) ? l : length;
    this->renew(l);
    for (int i = 0; i < lmin ; i++) {
      elements[i] = old.elements[i];
    }
  }
}

