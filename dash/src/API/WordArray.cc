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

#ifndef DAQHWYAPI_WORDARRAY_H
#include <dshapi/WordArray.h>
#endif

using namespace daqhwyapi;

/*==============================================================*/
/** @fn WordArray::WordArray()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
WordArray::WordArray() {
  elements = NULL;
  length = 0;
}

/*==============================================================*/
/** @fn WordArray::WordArray(const WordArray& rArry)
* @brief Copy constructor.
*
* Copy constructor. 
*
* @param rArry The other array.
* @return this
*/                                                             
WordArray::WordArray(const WordArray& rArry) {
  elements = NULL;
  length = 0;
  copyToThis(rArry);
}

/*==============================================================*/
/** @fn WordArray::WordArray(unsigned long l)
* @brief Constructor with size.
*
* Constructor with size.
*
* @param l The length of the array.
* @return this
*/                                                             
WordArray::WordArray(unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new uword[length];
  for (i = 0; i < length; i++) elements[i] = ' '; 
}

/*==============================================================*/
/** @fn WordArray::WordArray(const uword pElems[],unsigned long l)
* @brief Constructor with elements.
*
* Constructor with elements.
*
* @param pElems The elemets to use for construction.
* @param l The length of pElemes.
* @return this
*/                                                             
WordArray::WordArray(const uword pElems[],unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new uword[length];
  if (pElems != NULL) for (i = 0; i < length; i++) elements[i] = pElems[i];
  else memset((void*)elements,'\0',length);
}

/*==============================================================*/
/** @fn WordArray::~WordArray() 
* @brief Destructor.
*
* Destory this array.
*
* @param None
* @return None
*/                                                             
WordArray::~WordArray() {
  clear();
}

/*==============================================================*/
/** @fn void WordArray::clear() 
* @brief Empty this array.
*
* Empty this array.
*
* @param None
* @return None
*/                                                             
void WordArray::clear() {
  if (elements != NULL) {
    delete[] elements;
    elements = NULL;
  }
  length = 0; 
}

/*==============================================================*/
/** @fn WordArray& WordArray::operator=(const WordArray& ary) 
* @brief Assign an array to this one.
*
* Assign another WordArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
WordArray& WordArray::operator=(const WordArray& ary) {
  copyToThis(ary);
  return(*this);
} 

/*==============================================================*/
/** @fn void WordArray::copyToThis(const WordArray& ary) 
* @brief Assign an array to this one.
*
* Assign another WordArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
void WordArray::copyToThis(const WordArray& ary) {
  int i = 0;
  clear();
  length = ary.length;
  elements = new uword[length];
  if (ary.elements != NULL) for (i = 0; i < length; i++) elements[i] = ary.elements[i];
  else memset((void*)elements,'\0',length);
} 

/*==============================================================*/
/** @fn void WordArray::renew(unsigned long l) 
* @brief Empty this array and resize it.
*
* Empty this array and resize it.  Destroys existing 
* elements.
*
* @param l New size of this array.
* @return None
*/                                                             
void WordArray::renew(unsigned long l) {
  if (length != l) {
    clear();
    length = l;
    elements = new uword[length];
  }
  memset((void*)elements,'\0',length);
}
