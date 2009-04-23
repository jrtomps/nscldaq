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

#ifndef DAQ_OBJECTARRAY_H
#include <DAQObjectArray.h>
#endif

/*==============================================================*/
/** @fn DAQObjectArray::DAQObjectArray()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
DAQObjectArray::DAQObjectArray() {
  elements = NULL;
  length = 0;
}

/*==============================================================*/
/** @fn DAQObjectArray::DAQObjectArray(const DAQObjectArray& rArry)
* @brief Copy constructor.
*
* Copy constructor.
*
* @param rArry The other array.
* @return this
*/                                                             
DAQObjectArray::DAQObjectArray(const DAQObjectArray& rArry) {
  elements = NULL;
  length = 0;
  copyToThis(rArry);
}

/*==============================================================*/
/** @fn DAQObjectArray::DAQObjectArray(unsigned long l)
* @brief Constructor with a size.
*
* Constructor with a size.
*
* @param l The length of the array.
* @return this
*/                                                             
DAQObjectArray::DAQObjectArray(unsigned long l) {
  elements = NULL;
  length = l;
  elements = new DAQObject*[length];
  for (int i = 0; i < length; i++) elements[i] = NULL;
}

/*==============================================================*/
/** @fn DAQObjectArray::DAQObjectArray(Object* pElems[],unsigned long l)
* @brief Constructor with elements.
*
* Constructor with elements.
*
* @param pElems The elemets to use for construction.
* @param l The length of pElemes.
* @return this
*/                                                             
DAQObjectArray::DAQObjectArray(DAQObject* pElems[],unsigned long l) {
  elements = NULL;
  length = l;
  elements = new DAQObject*[length];
  if (pElems != NULL) {
    for (int i = 0; i < length; i++) {
      elements[i] = pElems[i];
    }
  } else {
    for (int i = 0; i < length; i++) elements[i] = NULL;
  }
}

/*==============================================================*/
/** @fn DAQObjectArray::~DAQObjectArray() 
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
DAQObjectArray::~DAQObjectArray() {
  clear();
}

/*==============================================================*/
/** @fn void DAQObjectArray::clear() 
* @brief Clear this array.
*
* Clear this array.
*
* @param None
* @return None
*/                                                             
void DAQObjectArray::clear() {
  if (elements != NULL) {
    delete[] elements;
    elements = NULL;
  }
  length = 0; 
}

/*==============================================================*/
/** @fn void DAQObjectArray::clearAndDelete() 
* @brief Clear this array and delete the elements.
*
* Clear this array and delete the elements.
*
* @param None
* @return None
*/                                                             
void DAQObjectArray::clearAndDelete() {
  if (elements != NULL) {
    for (int i = 0; i < length; i++) {
      if (elements[i] != NULL) {
        delete elements[i];
        elements[i] = NULL;
      }
    }
    delete[] elements;
    elements = NULL;
  }
  length = 0; 
}

/*==============================================================*/
/** @fn DAQObjectArray& DAQObjectArray::operator=(const DAQObjectArray& ary) 
* @brief Assign an array to this one.
*
* Assign another DAQObjectArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
DAQObjectArray& DAQObjectArray::operator=(const DAQObjectArray& ary) {
  copyToThis(ary);
  return(*this);
} 

/*==============================================================*/
/** @fn void DAQObjectArray::copyToThis(const DAQObjectArray& ary) 
* @brief Assign an array to this one.
*
* Assign another DAQObjectArray to this one.
*
* @param ary The other array.
* @return None
*/                                                             
void DAQObjectArray::copyToThis(const DAQObjectArray& ary) {
  clear();
  length = ary.length;
  elements = new DAQObject*[length];
  if (ary.elements != NULL) {
    for (int i = 0; i < length; i++) {
      elements[i] = ary.elements[i];
    }
  } else {
    for (int i = 0; i < length; i++) elements[i] = NULL;
  }
} 

/*==============================================================*/
/** @fn void DAQObjectArray::renew(unsigned long l) 
* @brief Empty this array and resize it.
*
* Empty this array and resize it.  Destroys existing 
* elements.
*
* @param l New size of this array.
* @return None
*/                                                             
void DAQObjectArray::renew(unsigned long l) {
  if (l == length) {
    if (elements != NULL) {
      for (int i = 0; i < length; i++) elements[i] = NULL;
    }
  } else {
    clear();
    length = l;
    elements = new DAQObject*[length];
    for (int i = 0; i < length; i++) elements[i] = NULL; 
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
void DAQObjectArray::resize(unsigned long l) {
  if (l != length) {
    DAQObjectArray old(*this);
    int lmin = (l < length) ? l : length;
    this->renew(l);
    for (int i = 0; i < lmin ; i++) {
      elements[i] = old.elements[i];
    }
  }
}

