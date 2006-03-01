#ifndef DAQHWYAPI_FILEOUTPUTSTREAM_H
#define DAQHWYAPI_FILEOUTPUTSTREAM_H

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

#ifndef DAQHWYAPI_FDOUTPUTSTREAM_H
#include <dshapi/FdOutputStream.h>
#endif

#ifndef DAQHWYAPI_STRING_H
#include <dshapi/String.h>
#endif

namespace daqhwyapi {

/**
* @class FileOutputStream
* @brief FileOutputStream for constructing streams from files.
*
* A class for constructing an input stream from a disk file.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class FileOutputStream : public FdOutputStream {
  public: 
    static int ModeTruncate;
    static int ModeAppend;
    static int DefaultPerms;

    FileOutputStream(std::string&); // Constructor with filename
    FileOutputStream(String&); // Constructor with filename
    FileOutputStream(String&,int); // Constructor with filename and mode
    FileOutputStream(std::string&,int); // Constructor with filename and mode
    FileOutputStream(FILE*); // Constructor with FILE pointer
    FileOutputStream(int); // Constructor with a file descriptor
    virtual ~FileOutputStream();  // Destructor

  protected:
    FileOutputStream(); // Default constructor 
    int write_open(const char*,int,mode_t); // Open a file for writing
};

} // namespace daqhwyapi

#endif
