/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_STRINGARRAY_H
#include <dshapi/StringArray.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

namespace daqhwyapi {
/**
* @var stringarray_indexoutofbounds_exception
* @brief Exception to throw when an array index does not exist.
*
* Exception to throw when an array index does not exist.
*/
static IndexOutOfBoundsException stringarray_indexoutofbounds_exception;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*==============================================================*/
/** @fn StringArray::StringArray()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
StringArray::StringArray() {
  elements = NULL;
  length = 0;
}

/*==============================================================*/
/** @fn StringArray::StringArray(const StringArray& rArry)
* @brief Copy constructor.
*
* Copy constructor.
*
* @param rArry The other array.
* @return this
*/                                                             
StringArray::StringArray(const StringArray& rArry) {
  elements = NULL;
  length = 0;
  copyToThis(rArry);
}

/*==============================================================*/
/** @fn StringArray::StringArray(unsigned long l)
* @brief Constructor with a size.
*
* Constructor with a size.
*
* @param l The length of the array.
* @return this
*/                                                             
StringArray::StringArray(unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new String*[length];
  for (i = 0; i < length; i++) elements[i] = NULL;
}

/*==============================================================*/
/** @fn StringArray::StringArray(String* pElems[],unsigned long l)
* @brief Constructor with elements.
*
* Constructor with elements.
*
* @param pElems The elemets to use for construction.
* @param l The length of pElemes.
* @return this
*/                                                             
StringArray::StringArray(String* pElems[],unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new String*[length];
  if (pElems != NULL) {
    for (i = 0; i < length; i++) {
      elements[i] = pElems[i];
    }
  } else {
    for (i = 0; i < length; i++) elements[i] = NULL;
  }
}

/*==============================================================*/
/** @fn StringArray::~StringArray() 
* @brief Destructor.
*
* Destroy this object.
*
* @param None
* @return None
*/                                                             
StringArray::~StringArray() {
  clear();
}

/*==============================================================*/
/** @fn void StringArray::clear() 
* @brief Clear this array.
*
* Clear this array.
*
* @param None
* @return None
*/                                                             
void StringArray::clear() {
  if (elements != NULL) {
    delete[] elements;
    elements = NULL;
  }
  length = 0; 
}

/*==============================================================*/
/** @fn void StringArray::clearAndDelete() 
* @brief Clear this array and delete the elements.
*
* Clear this array and delete the elements.
*
* @param None
* @return None
*/                                                             
void StringArray::clearAndDelete() {
  if (elements != NULL) {
    int i = 0;
    for (i = 0; i < length; i++) {
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
/** @fn StringArray& StringArray::operator=(const StringArray& ary) 
* @brief Assign an array to this one.
*
* Assign another StringArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
StringArray& StringArray::operator=(const StringArray& ary) {
  copyToThis(ary);
  return(*this);
} 

/*==============================================================*/
/** @fn void StringArray::copyToThis(const StringArray& ary) 
* @brief Assign an array to this one.
*
* Assign another StringArray to this one.
*
* @param ary The other array.
* @return None
*/                                                             
void StringArray::copyToThis(const StringArray& ary) {
  int i = 0;
  clear();
  length = ary.length;
  elements = new String*[length];
  if (ary.elements != NULL) {
    for (i = 0; i < length; i++) {
      elements[i] = ary.elements[i];
    }
  } else {
    for (i = 0; i < length; i++) elements[i] = NULL;
  }
} 

/*==============================================================*/
/** @fn void StringArray::renew(unsigned long l) 
* @brief Empty this array and resize it.
*
* Empty this array and resize it.  Destroys existing 
* elements.
*
* @param l New size of this array.
* @return None
*/                                                             
void StringArray::renew(unsigned long l) {
  if (l == length) {
    if (elements != NULL) {
      for (int i = 0; i < length; i++) elements[i] = NULL;
    }
  } else {
    clear();
    length = l;
    elements = new String*[length];
    for (int i = 0; i < length; i++) elements[i] = NULL; 
  }
}
