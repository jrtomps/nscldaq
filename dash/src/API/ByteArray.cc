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

#ifndef DAQHWYAPI_BYTEARRAY_H
#include <dshapi/ByteArray.h>
#endif

using namespace daqhwyapi;

/*==============================================================*/
/** @fn ByteArray::ByteArray()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
ByteArray::ByteArray() {
  elements = NULL;
  length = 0;
}

/*==============================================================*/
/** @fn ByteArray::ByteArray(const ByteArray& rArry)
* @brief Copy constructor.
*
* Copy constructor. 
*
* @param rArry The other array.
* @return this
*/                                                             
ByteArray::ByteArray(const ByteArray& rArry) {
  elements = NULL;
  length = 0;
  copyToThis(rArry);
}

/*==============================================================*/
/** @fn ByteArray::ByteArray(unsigned long l)
* @brief Constructor with size.
*
* Constructor with size.
*
* @param l The length of the array.
* @return this
*/                                                             
ByteArray::ByteArray(unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new ubyte[length];
  for (i = 0; i < length; i++) elements[i] = ' '; 
}

/*==============================================================*/
/** @fn ByteArray::ByteArray(const ubyte pElems[],unsigned long l)
* @brief Constructor with elements.
*
* Constructor with elements.
*
* @param pElems The elemets to use for construction.
* @param l The length of pElemes.
* @return this
*/                                                             
ByteArray::ByteArray(const ubyte pElems[],unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new ubyte[length];
  if (pElems != NULL) for (i = 0; i < length; i++) elements[i] = pElems[i];
  else memset((void*)elements,'\0',length);
}

/*==============================================================*/
/** @fn ByteArray::~ByteArray() 
* @brief Destructor.
*
* Destory this array.
*
* @param None
* @return None
*/                                                             
ByteArray::~ByteArray() {
  clear();
}

/*==============================================================*/
/** @fn void ByteArray::clear() 
* @brief Empty this array.
*
* Empty this array.
*
* @param None
* @return None
*/                                                             
void ByteArray::clear() {
  if (elements != NULL) {
    delete[] elements;
    elements = NULL;
  }
  length = 0; 
}

/*==============================================================*/
/** @fn ByteArray& ByteArray::operator=(const ByteArray& ary) 
* @brief Assign an array to this one.
*
* Assign another ByteArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
ByteArray& ByteArray::operator=(const ByteArray& ary) {
  copyToThis(ary);
  return(*this);
} 

/*==============================================================*/
/** @fn void ByteArray::copyToThis(const ByteArray& ary) 
* @brief Assign an array to this one.
*
* Assign another ByteArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
void ByteArray::copyToThis(const ByteArray& ary) {
  int i = 0;
  clear();
  length = ary.length;
  elements = new ubyte[length];
  if (ary.elements != NULL) for (i = 0; i < length; i++) elements[i] = ary.elements[i];
  else memset((void*)elements,'\0',length);
} 

/*==============================================================*/
/** @fn void ByteArray::renew(unsigned long l) 
* @brief Empty this array and resize it.
*
* Empty this array and resize it.  Destroys existing 
* elements.
*
* @param l New size of this array.
* @return None
*/                                                             
void ByteArray::renew(unsigned long l) {
  if (length != l) {
    clear();
    length = l;
    elements = new ubyte[length];
  }
  memset((void*)elements,'\0',length);
}

