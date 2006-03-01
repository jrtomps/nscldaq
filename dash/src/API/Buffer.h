#ifndef DAQHWYAPI_BUFFER_H
#define DAQHWYAPI_BUFFER_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#ifndef DAQHWYAPI_EXCEPTIONS_H
#include <dshapi/Exceptions.h>
#endif

#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_MAINDEFS_H
#include <dshapi/maindefs.h>
#endif

namespace daqhwyapi {

/**
* @class Buffer
* @brief Buffer definition.
*
* The definition for the Buffer class implementing dynamic
* buffering.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Buffer : public Object {
  public: 
    virtual ~Buffer();  // Destructor 
    void clear(); // Empty this Buffer
    int capacity();   // Return the size (number of elements)
    int remaining(); // Return the number of remaining elements
    bool rewind();   // Reset buffer to buffer start.
    bool reset();   // Reset buffer to previous mark.
    bool mark();   // Mark current position.
    int  limit();  // Get the buffer limit (max size)
    bool limit(int);  // Set the buffer limit (max size)
    int position();  // Get the buffer position.
    bool position(int);  // Set the buffer position.
    virtual bool consume(int) = 0;  // Consume start of buffer.
    virtual bool truncate(int) = 0;  // Keep only the first n elements.

  protected:
    Buffer();          // Default constructor 
    void initialize(); // Initialize this object
    void *grow(int,int);   // Grow this Buffer

    /**
    * @var vals
    * @brief Array values.
    *
    * Pointer to a simple array of Object values.
    */
    ubyte *vals; 

    /**
    * @var scnt
    * @brief Current length of storage requested for allocation.
    *
    * Current length of storage requested for allocation in
    * units of primitive type units.
    */
    int scnt; 

    /**
    * @var cmax
    * @brief Current allocated buffer size.
    *
    * Current allocated buffer size in units of primitive type units.
    */
    int cmax; 

    /**
    * @var my_limit
    * @brief Maximum allowed size of this buffer.
    *
    * Maximum allowed size of this buffer.
    */
    int my_limit;

    /**
    * @var my_mark
    * @brief Position mark for implementing position operations.
    *
    * Position mark for implementing position sensitive operations.
    */
    int my_mark;

    /**
    * @var my_pos
    * @brief Current buffer position.
    *
    * Current buffer position.
    */
    int my_pos;

    /**
    * @var member_size
    * @brief Size of members stored in this buffer.
    *
    * Size of members stored in this buffer.
    */
    int member_size;
};


} // namespace daqhwyapi

#endif
