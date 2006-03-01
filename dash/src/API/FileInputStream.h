#ifndef DAQHWYAPI_FILEINPUTSTREAM_H
#define DAQHWYAPI_FILEINPUTSTREAM_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/


#ifndef DAQHWYAPI_OBJECT_H
#include <dshapi/Object.h>
#endif

#ifndef DAQHWYAPI_FDINPUTSTREAM_H
#include <dshapi/FdInputStream.h>
#endif

#ifndef DAQHWYAPI_BYTEBUFFER_H
#include <dshapi/ByteBuffer.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

namespace daqhwyapi {

/**
* @class FileInputStream
* @brief FileInputStream for constructing streams from files.
*
* A class for constructing an input stream from a disk file.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FileInputStream : public FdInputStream {
  public: 
    FileInputStream(std::string&); // Constructor with filename
    FileInputStream(String&); // Constructor with filename
    FileInputStream(FILE*); // Constructor with FILE pointer
    FileInputStream(int); // Constructor with a file descriptor
    virtual ~FileInputStream();  // Destructor

  protected:
    FileInputStream(); // Default constructor  
    int read_open(const char *fname);
};

} // namespace daqhwyapi

#endif
