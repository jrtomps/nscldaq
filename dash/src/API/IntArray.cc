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

#ifndef DAQHWYAPI_INTARRAY_H
#include <dshapi/IntArray.h>
#endif

using namespace daqhwyapi;

/*==============================================================*/
/** @fn IntArray::IntArray()
* @brief Default constructor.
*
* Default constructor.
*
* @param None
* @return this
*/                                                             
IntArray::IntArray() {
  elements = NULL;
  length = 0;
}

/*==============================================================*/
/** @fn IntArray::IntArray(const IntArray& rArry)
* @brief Copy constructor.
*
* Copy constructor. 
*
* @param rArry The other array.
* @return this
*/                                                             
IntArray::IntArray(const IntArray& rArry) {
  elements = NULL;
  length = 0;
  copyToThis(rArry);
}

/*==============================================================*/
/** @fn IntArray::IntArray(unsigned long l)
* @brief Constructor with size.
*
* Constructor with size.
*
* @param l The length of the array.
* @return this
*/                                                             
IntArray::IntArray(unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new int[length];
  for (i = 0; i < length; i++) elements[i] = 0; 
}

/*==============================================================*/
/** @fn IntArray::IntArray(const int pElems[],unsigned long l)
* @brief Constructor with elements.
*
* Constructor with elements.
*
* @param pElems The elemets to use for construction.
* @param l The length of pElemes.
* @return this
*/                                                             
IntArray::IntArray(const int pElems[],unsigned long l) {
  elements = NULL;
  length = l;
  int i = 0;
  elements = new int[length];
  if (pElems != NULL) for (i = 0; i < length; i++) elements[i] = pElems[i];
  else for (i = 0; i < length; i++) elements[i] = 0; 
}

/*==============================================================*/
/** @fn IntArray::~IntArray() 
* @brief Destructor.
*
* Destory this array.
*
* @param None
* @return None
*/                                                             
IntArray::~IntArray() {
  clear();
}

/*==============================================================*/
/** @fn void IntArray::clear() 
* @brief Empty this array.
*
* Empty this array.
*
* @param None
* @return None
*/                                                             
void IntArray::clear() {
  if (elements != NULL) {
    delete[] elements;
    elements = NULL;
  }
  length = 0; 
}

/*==============================================================*/
/** @fn IntArray& IntArray::operator=(const IntArray& ary) 
* @brief Assign an array to this one.
*
* Assign another IntArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
IntArray& IntArray::operator=(const IntArray& ary) {
  copyToThis(ary);
  return(*this);
} 

/*==============================================================*/
/** @fn void IntArray::copyToThis(const IntArray& ary) 
* @brief Assign an array to this one.
*
* Assign another IntArray to this one.
*
* @param ary The other array.
* @return A reference to this.
*/                                                             
void IntArray::copyToThis(const IntArray& ary) {
  int i = 0;
  clear();
  length = ary.length;
  elements = new int[length];
  if (ary.elements != NULL) for (i = 0; i < length; i++) elements[i] = ary.elements[i];
  else for (i = 0; i < length; i++) elements[i] = 0; 
} 

/*==============================================================*/
/** @fn void IntArray::renew(unsigned long l) 
* @brief Empty this array and resize it.
*
* Empty this array and resize it.  Destroys existing 
* elements.
*
* @param l New size of this array.
* @return None
*/                                                             
void IntArray::renew(unsigned long l) {
  if (length != l) {
    clear();
    length = l;
    elements = new int[length];
  }
  else for (int i = 0; i < length; i++) elements[i] = 0; 
}

