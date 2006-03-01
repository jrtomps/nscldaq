#ifndef DAQHWYAPI_PIPECONNECTOR_H
#define DAQHWYAPI_PIPECONNECTOR_H

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

#ifndef DAQHWYAPI_FDOUTPUTSTREAM_H
#include <dshapi/FdOutputStream.h>
#endif

namespace daqhwyapi {

/**
* @class PipeConnector
* @brief PipeConnector class definition.
*
* The PipeConnector class allows an Fd Input and
* Output stream to be connected (using a Unix pipe).
*
* @author  Eric Kasten
* @version 1.0.0
*/
class PipeConnector : public Object {
  public: 
    PipeConnector(); // Constructor 
    virtual ~PipeConnector(); // Destructor
    bool connect(FdInputStream&); // Connect an input stream
    bool connectInput(int); // Connect an input fd
    bool connect(FdOutputStream&); // Connect an output stream
    bool connectOutput(int); // Connect an output fd
    int getInputFD();  // Get the input fd
    int getOutputFD(); // Get the output fd
    void close();  // Close this pipe connector
    void open();  // Ready this connector to receive connections

  protected:
    void make_pipe(int fd[]);

  private:
    int pipefd[2];
};

} // namespace daqhwyapi

#endif
