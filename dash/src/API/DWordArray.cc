/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <string.h>

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_DWORDARRAY_H
#include <dshapi/DWordArray.h>
#endif

using namespace daqhwyapi;

/*==============================================================*/
/** @fn DWordArray::DWordArray()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
DWordArray::DWordArray() {
  elements = NULL;
  length = 0;
}

/*==============================================================*/
/** @fn DWordArray::DWordArray(const DWordArray& rArry)
* @brief Copy constructor.
*
* Copy constructor. 
*
* @param rArry The other array.
* @return this
*/                                                             
DWordArray::DWordArray(const DWordArray& rArry) {
  elements = NULL;
  length = 0;
  copyToThis(rArry);
}

/*==============================================================*/
/** @fn DWordArray::DWordArray(unsigned long l)
* @brief Constructor with size.
*
* Constructor with size.
*
* @param l The length of the array.
* @return this
*/                                                             
DWordArray::DWordArray(unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new duword[length];
  for (i = 0; i < length; i++) elements[i] = 0; 
}

/*==============================================================*/
/** @fn DWordArray::DWordArray(const duword pElems[],unsigned long l)
* @brief Constructor with elements.
*
* Constructor with elements.
*
* @param pElems The elemets to use for construction.
* @param l The length of pElemes.
* @return this
*/                                                             
DWordArray::DWordArray(const duword pElems[],unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new duword[length];
  if (pElems != NULL) for (i = 0; i < length; i++) elements[i] = pElems[i];
  else for (i = 0; i < length; i++) elements[i] = 0; 
}

/*==============================================================*/
/** @fn DWordArray::~DWordArray() 
* @brief Destructor.
*
* Destory this array.
*
* @param None
* @return None
*/                                                             
DWordArray::~DWordArray() {
  clear();
}

/*==============================================================*/
/** @fn void DWordArray::clear() 
* @brief Empty this array.
*
* Empty this array.
*
* @param None
* @return None
*/                                                             
void DWordArray::clear() {
  if (elements != NULL) {
    delete[] elements;
    elements = NULL;
  }
  length = 0; 
}

/*==============================================================*/
/** @fn DWordArray& DWordArray::operator=(const DWordArray& ary) 
* @brief Assign an array to this one.
*
* Assign another DWordArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
DWordArray& DWordArray::operator=(const DWordArray& ary) {
  copyToThis(ary);
  return(*this);
} 

/*==============================================================*/
/** @fn void DWordArray::copyToThis(const DWordArray& ary) 
* @brief Assign an array to this one.
*
* Assign another DWordArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
void DWordArray::copyToThis(const DWordArray& ary) {
  int i = 0;
  clear();
  length = ary.length;
  elements = new duword[length];
  if (ary.elements != NULL) for (i = 0; i < length; i++) elements[i] = ary.elements[i];
  else for (i = 0; i < length; i++) elements[i] = 0; 
} 

/*==============================================================*/
/** @fn void DWordArray::renew(unsigned long l) 
* @brief Empty this array and resize it.
*
* Empty this array and resize it.  Destroys existing 
* elements.
*
* @param l New size of this array.
* @return None
*/                                                             
void DWordArray::renew(unsigned long l) {
  if (length != l) {
    clear();
    length = l;
    elements = new duword[length];
  }
  else for (int i = 0; i < length; i++) elements[i] = 0; 
}

