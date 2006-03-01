/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

using namespace std;

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <ios>
#include <iosfwd>

#ifndef DAQHWYAPI_CSTR_H
#include <dshapi/cstr.h>
#endif

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

#ifndef DAQHWYAPI_SYSTEM_H
#include <dshapi/System.h>
#endif

#ifndef DAQHWYAPI_BYTEBUFFER_H
#include <dshapi/ByteBuffer.h>
#endif

namespace daqhwyapi {
/**
* @var ubytebuffer_bad_alloc
* @brief Exception to throw when allocation fails.
*
* Exception to throw when allocation fails.
*/
static BadMemoryAllocationException ubytebuffer_bad_alloc;

/**
* @var ubytebuffer_out_of_bounds
* @brief Exception to throw for indexing errors.
*
* Exception to throw for indexing errors.
*/
static IndexOutOfBoundsException ubytebuffer_out_of_bounds;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*===================================================================*/
/** @fn ByteBuffer::ByteBuffer()
* @brief Default constructor.
*                                        
* Default constructor.  
*                                         
* @param None
* @return this                 
*/      
ByteBuffer::ByteBuffer() { 
  member_size = 1;
}

/*===================================================================*/
/** @fn ByteBuffer::ByteBuffer(const ubyte *s,int len)
* @brief Constructor with ubyte pointer and length.
*                                        
* Constructor with ubyte pointer and length.
*                                         
* @param s The ubyte pointer.
* @param len The number of ubytes.
* @return this                 
*/      
ByteBuffer::ByteBuffer(const ubyte *s,int len) { 
  member_size = 1;
  if (len > limit()) limit(len);
  if (s != NULL) put(0,(ubyte*)s,len);
}

/*===================================================================*/
/** @fn ByteBuffer::ByteBuffer(const ByteBuffer& s)
* @brief Copy constructor.
*                                        
* Copy constructor.
*                                         
* @param s The ubytebuffer.
* @return this                 
*/      
ByteBuffer::ByteBuffer(const ByteBuffer& s) { 
  member_size = 1;
  if (s.cmax > cmax) limit(s.cmax);
  put(0,s.vals,s.scnt);
}

/*===================================================================*/
/** @fn ByteBuffer::~ByteBuffer()
* @brief Destructor.
*                                        
* Destructor method for this class. 
*                                         
* @param None
* @return None 
*/      
ByteBuffer::~ByteBuffer() { 
  clear();
}

/*==============================================================*/
/** @fn bool ByteBuffer::put(int idx,ubyte *s,int len) 
* @brief Absolute put method.
*
* Absolute put method that adds ubytes to this buffer at the
* specified index.  The current position does not change.
*
* @param idx The buffer index.
* @param s The ubyte pointer.
* @param len The number of ubytes to append.
* @return true if the ubytebuffer has changed. 
*/                                                             
bool ByteBuffer::put(int idx,ubyte *s,int len) {
  ubyte *p = NULL;
  if (s == NULL) return false;
  if (len <= 0) return false;
  if (idx < 0) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::put() out of bounds at index %d"),idx);
  if ((idx+len) > cmax) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::put() out of bounds at index %d"),(idx+len));
  p = &(vals[idx]);
  memcpy(p,s,len);
  if ((idx+len) > scnt) scnt = (idx+len);
  return true;
}

/*==============================================================*/
/** @fn bool ByteBuffer::put(int idx,ubyte s) 
* @brief Absolute put method.
*
* Absolute put method that adds ubytes to this buffer at the
* specified index.  The current position does not change.
*
* @param idx The buffer index.
* @param s The ubyte.
* @return true if the ubytebuffer has changed. 
*/                                                             
bool ByteBuffer::put(int idx,ubyte s) {
  if ((idx < 0)||(idx >= cmax)) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::put() out of bounds at index %d"),idx);
  vals[idx] = s;
  if (idx >= scnt) scnt = idx+1;
  return true;
}

/*==============================================================*/
/** @fn ubyte *ByteBuffer::get(int idx,ubyte *outVal,int len) 
* @brief Absolute get method.
*
* Absolute get method that gets ubytes from this buffer at the
* specified index.  The current position does not change.
*
* @param idx The buffer index.
* @param outVal The output ubytes.
* @param len The number of ubytes to append.
* @return A pointer to the output ubytes or NULL.
*/                                                             
ubyte *ByteBuffer::get(int idx,ubyte *outVal,int len) {
  ubyte *p = NULL;
  if ((len <= 0)||(outVal == NULL)) return NULL;
  if (idx < 0) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::get() out of bounds at index %d"),idx);
  if ((idx+len) > scnt) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::get() index greater than capacity of buffer %d"),(idx+len));
  p = &(vals[idx]);
  memcpy(outVal,p,len);
  return outVal;
}

/*==============================================================*/
/** @fn ubyte ByteBuffer::get(int idx) 
* @brief Absolute get method.
*
* Absolute get method that gets a ubyte from this buffer at the
* specified index.  The current position does not change.
*
* @param idx The buffer index.
* @return The ubyte.
*/                                                             
ubyte ByteBuffer::get(int idx) {
  if (idx < 0) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::get() out of bounds at index %d"),idx);
  if (idx >= scnt) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::get() index greater than capacity of buffer %d"),idx);
  ubyte s = vals[idx];
  return s;
}

/*==============================================================*/
/** @fn bool ByteBuffer::put(ubyte *s,int len) 
* @brief Relative put method.
*
* Relative put method that adds ubytes to this buffer at the
* current position.  The current position is incremented by len.
*
* @param s The ubyte pointer.
* @param len The number of ubytes to append.
* @return true if the ubytebuffer has changed. 
*/                                                             
bool ByteBuffer::put(ubyte *s,int len) {
  ubyte *p = NULL;
  if (s == NULL) return false;
  if (len <= 0) return false;
  if ((my_pos+len) > cmax) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::put() put would exceed buffer limit"));
  p = &(vals[my_pos]);
  memcpy(p,s,len);
  my_pos += len;
  if (my_pos >= scnt) scnt = my_pos;
  return true;
}

/*==============================================================*/
/** @fn bool ByteBuffer::put(ubyte s) 
* @brief Relative put method.
*
* Relative put method that adds a ubyte to this buffer at the
* current position.  The current position is incremented.
*
* @param s The ubyte.
* @return true if the ubytebuffer has changed. 
*/                                                             
bool ByteBuffer::put(ubyte s) {
  if (my_pos >= cmax) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::put() current position greater than capacity of buffer %d"),my_pos);
  vals[my_pos] = s; 
  my_pos++;
  if (my_pos >= scnt) scnt = my_pos;
  return true;
}

/*==============================================================*/
/** @fn ubyte *ByteBuffer::get(ubyte *outVal,int len) 
* @brief Relative get method.
*
* Relative get method that gets ubytes from this buffer at the
* current position.  The current position is incremented by len.
*
* @param outVal The output ubytes.
* @param len The number of ubytes to get.
* @return A pointer to the output ubytes or NULL.
*/                                                             
ubyte *ByteBuffer::get(ubyte *outVal,int len) {
  ubyte *p = NULL;
  if ((len <= 0)||(outVal == NULL)) return NULL;
  if ((my_pos+len) > scnt) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::get() index greater than capacity of buffer %d"),(my_pos+len));
  p = &(vals[my_pos]);
  memcpy(outVal,p,len);
  my_pos += len;
  scnt -= len; 
  return outVal;
}

/*==============================================================*/
/** @fn ubyte ByteBuffer::get() 
* @brief Relative get method.
*
* Relative get method that gets a ubyte from this buffer at the
* current position.  The current position is incremented.
*
* @param None
* @return The ubyte.
*/                                                             
ubyte ByteBuffer::get() {
  if (my_pos >= scnt) throw ubytebuffer_out_of_bounds.format(CSTR("ByteBuffer::get() index greater than capacity of buffer %d"),my_pos);
  ubyte s = vals[my_pos]; 
  my_pos++;
  scnt--;
  return s;
}

/*==============================================================*/
/** @fn bool ByteBuffer::truncate(int cnt) 
* @brief Truncate this buffer to the specified number of ubytes.
*
* Truncate this buffer to the specified number of ubytes.
*
* @param cnt The number of ubytes to keep.
* @return true if the ubytebuffer has changed. 
*/                                                             
bool ByteBuffer::truncate(int cnt) {
  if (cnt >= scnt) return false;
  scnt = cnt;
  return true;
}

/*==============================================================*/
/** @fn bool ByteBuffer::consume(int len) 
* @brief Consume ubytes at start of buffer.
*
* Consume ubytes at start of buffer.
*
* @param cnt The number of ubytes to consume.
* @return true if the ubytebuffer has changed. 
*/                                                             
bool ByteBuffer::consume(int len) {
  if (len <= 0) return false;
  if (scnt >= len) memmove(vals,vals+len,(scnt-len));
  int l = (len > scnt) ? scnt : len;
  scnt -= l; if (scnt < 0) my_pos = 0;
  my_pos -= l; if (my_pos < 0) my_pos = 0;
  my_mark -= l; if (my_mark < 0) my_mark = 0;
  return true;
}

/*==============================================================*/
/** @fn ubyte *ByteBuffer::array(ubyte *outVal,int& len) 
* @brief Get a ubyte pointer representation.
*
* Get a ubyte pointer representation.
*
* @param outVal The output array.
* @param len On input the length of outVal. On output, the length in bytes of the array returned.
* @return The ubyte pointer representation.
*/                                                             
ubyte *ByteBuffer::array(ubyte *outVal,int& len) {
  int l = scnt * member_size;
  l = (l > len) ? len : l;
  len = l;
  memcpy(outVal,vals,l);
  return outVal;  
}

