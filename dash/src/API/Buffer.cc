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

#ifndef DAQHWYAPI_BUFFER_H
#include <dshapi/Buffer.h>
#endif

namespace daqhwyapi {
/**
* @var dynamicbuffer_bad_alloc
* @brief Exception to throw when allocation fails.
*
* Exception to throw when allocation fails.
*/
static BadMemoryAllocationException dynamicbuffer_bad_alloc;

/**
* @var dynamicbuffer_out_of_bounds
* @brief Exception to throw for indexing errors.
*
* Exception to throw for indexing errors.
*/
static IndexOutOfBoundsException dynamicbuffer_out_of_bounds;
} // namespace daqhwyapi

using namespace daqhwyapi;

/*===================================================================*/
/** @fn Buffer::Buffer()
* @brief Default constructor.
*                                        
* Default constructor.  
*                                         
* @param None
* @return this                 
*/      
Buffer::Buffer() { 
  initialize();
}

/*===================================================================*/
/** @fn void Buffer::initialize()
* @brief initialize.
*                                        
* initialize.
*                                         
* @param None
* @return None
*/      
void Buffer::initialize() { 
  vals = NULL;
  scnt = 0;
  cmax = 0;
  member_size = 1;
  my_mark = 0;
  my_pos = 0;
  my_limit = 0;
  limit(1024);
}

/*===================================================================*/
/** @fn Buffer::~Buffer()
* @brief Destructor.
*                                        
* Destructor method for this class. 
*                                         
* @param None
* @return None 
*/      
Buffer::~Buffer() { 
  if (vals != NULL) delete[] vals;
  vals = NULL;
}

/*==============================================================*/
/** @fn void clear() 
* @brief Empty this dynamicbuffer.
*
* Empty this dynamicbuffer.
*
* @param None
* @return None
*/                                                             
void Buffer::clear() {
  scnt = 0;
  my_mark = 0;
  my_pos = 0;
}

/*==============================================================*/
/** @fn int Buffer::capacity()
* @brief Return the length of this dynamicbuffer.
*
* Return the length of this dynamicbuffer.
*
* @param None
* @return The length of this dynamicbuffer.
*/                                                             
int Buffer::capacity() {
  return scnt;
}

/*==============================================================*/
/** @fn int Buffer::remaining()
* @brief Return the number of remaining elements.
*
* Return the number of remaining elements between the current
* position and the limit.
*
* @param None
* @return The number of remaining elements.
*/                                                             
int Buffer::remaining() {
  return my_limit - my_pos;
}

/*==============================================================*/
/** @fn bool Buffer::rewind()
* @brief Rewind the buffer to the buffer start.
*
* Rewind the buffer to the buffer start.
*
* @param None
* @return If the buffer was rewound.
*/                                                             
bool Buffer::rewind() {
  my_pos = 0;
  return true;
}

/*==============================================================*/
/** @fn bool Buffer::reset()
* @brief Reset the current position to a previous mark.
*
* Reset the current position to a previous mark.  If no mark
* has been establish, position will be set to 0. 
*
* @param None
* @return If the buffer was reset.
*/                                                             
bool Buffer::reset() {
  my_pos = my_mark;
  return true;
}

/*==============================================================*/
/** @fn bool Buffer::mark()
* @brief Establish a mark.
*
* Establish a mark at the current position.     
*
* @param None
* @return If the buffer was marked.
*/                                                             
bool Buffer::mark() {
  my_mark = my_pos;
  return true;
}

/*==============================================================*/
/** @fn int Buffer::limit()
* @brief Get the current limit.
*
* Get the current limit.  The limit is the maximum buffer size.
*
* @param None
* @return The current limit.
*/                                                             
int Buffer::limit() {
  return my_limit;
}

/*==============================================================*/
/** @fn bool Buffer::limit(int aLimit)
* @brief Set the current limit.
*
* Set the current limit.  The limit is the maximum buffer size.
* A limit of zero implies an unlimited buffer size.
*
* @param aLimit The new limit.
* @return If a limit was set.
*/                                                             
bool Buffer::limit(int aLimit) {
  if (aLimit < 0) return false;
  int old_limit = my_limit;
  my_limit = aLimit;
  if (grow(my_limit,member_size) == NULL) throw(dynamicbuffer_bad_alloc.format(CSTR("Buffer::limit() cannot allocate memory of size = %d"),(my_limit*member_size)));
  if (my_limit < old_limit) {
    if (scnt > my_limit) scnt = my_limit; 
    if (my_pos >= my_limit) my_pos = my_limit-1;
    if (my_mark >= my_limit) my_mark = my_limit-1;
  }
  return true;
}

/*==============================================================*/
/** @fn int Buffer::position()
* @brief Get the current buffer position.
*
* Get the current buffer position.
*
* @param None
* @return The current position.
*/                                                             
int Buffer::position() {
  return my_pos;
}

/*==============================================================*/
/** @fn bool Buffer::position(int aPos)
* @brief Set the current buffer position.
*
* Set the current buffer position.
*
* @param aPos The new position.
* @return If the position was set.
* @throw IndexOutOfBoundsException If the position is invalid.
*/                                                             
bool Buffer::position(int aPos) {
  if ((aPos < 0)||(aPos >= my_limit)) throw dynamicbuffer_out_of_bounds.format(CSTR("Buffer::position() position is out-of-bounds at position=%d"),aPos);
  my_pos = aPos;
  return true;
}

/*==============================================================*/
/** @fn void *Buffer::grow(int nmemb,int siz)
* @brief Grow this Buffer.
*
* Grow this Buffer to the specified size.  
*
* @param nmemb Number of members.
* @param siz Size of a member.
* @return The allocated array or NULL on failure.
*/                                                             
void *Buffer::grow(int nmemb,int siz) {
  ubyte *ns = NULL;
  int ubytesiz = (siz * nmemb);

  if (nmemb > my_limit) throw(dynamicbuffer_bad_alloc.format(CSTR("Buffer::grow() cannot allocate buffer greater than established limit (%d > %d)"),nmemb,my_limit));

  if (ubytesiz <= 0) ubytesiz = 1;

  if (vals == NULL) { // Init
    try {
      vals = new ubyte[ubytesiz];
      memset(vals,0,ubytesiz);
    } catch (...) {
      return(NULL);
    }
    cmax = nmemb;
  } else if (nmemb > cmax) { // Grow 
    try {
      ns = new ubyte[ubytesiz]; 
      memset(ns,0,ubytesiz);
    } catch (...) {
      return(NULL);
    }
    if (vals != NULL) {
      memcpy(ns,vals,(scnt*siz));
      delete[] vals;
    }
    vals = ns;
    cmax = nmemb;
  } else if (nmemb < cmax) {  // Shrink
    try {
      ns = new ubyte[ubytesiz]; 
      memset(ns,0,ubytesiz);
    } catch (...) {
      return(NULL);
    }
    if (vals != NULL) {
      memcpy(ns,vals,(nmemb*siz));
      delete[] vals;
    }
    vals = ns;
    cmax = nmemb;
  } 

  return((void*)vals);
}

