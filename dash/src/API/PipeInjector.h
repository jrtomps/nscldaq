#ifndef DAQHWYAPI_PIPEINJECTOR_H
#define DAQHWYAPI_PIPEINJECTOR_H

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

#ifndef DAQHWYAPI_RUNNABLE_H
#include <dshapi/Runnable.h>
#endif

#ifndef DAQHWYAPI_FDINPUTSTREAM_H
#include <dshapi/FdInputStream.h>
#endif

#ifndef DAQHWYAPI_FDOUTPUTSTREAM_H
#include <dshapi/FdOutputStream.h>
#endif

#ifndef DAQHWYAPI_FDRECORDREADER_H
#include <dshapi/FdRecordReader.h>
#endif

#ifndef DAQHWYAPI_RECORD_H
#include <dshapi/Record.h>
#endif

namespace daqhwyapi {

/**
* @class PipeInjector
* @brief PipeInjector class definition.
*
* The PipeInjector class allows an a Reader to inject
* records into the main stream of the pipeline.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class PipeInjector : public Runnable { 
  public: 
    PipeInjector(); // Constructor 
    virtual ~PipeInjector(); // Destructor
    bool connect(FdInputStream&); // Connect an input stream
    bool connect(FdOutputStream&); // Connect an output stream
    bool connect(FdRecordReader&); // Connect an injector
    bool connect(FdInputStream&,FdOutputStream&); // Connect an in/out error stream pair. 
    void start(); // Start main loop of the injector (Runnable interface)
    void run(); // Main loop of the injector (Runnable interface)
    void setRecordType(uint32_t); // Set the output record type

  protected:
    FdInputStream *mainin;
    FdOutputStream *mainout;
    FdRecordReader *injector;
    FdInputStream *errorin;
    FdOutputStream *errorout;
    bool ignore_injector;
    uint32_t rec_type;

    void check_injector(); 
    int check_maininput(record_header_t&);
    void check_errorinput(); 
};

} // namespace daqhwyapi

#endif
