/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CDeviceCommand.h
# @brief  base class that provides utilities for VMUSB device support cmd classes.
# @author <fox@nscl.msu.edu>
*/

#ifndef CDEVICECOMMAND_H
#define CDEVICECOMMAND_H

#include <TCLObjectProcessor.h>


// forward class definitions

class CTCLInterpreter;
class CTCLObject;
class CReadoutModule;
class CConfiguration;

/**
 * @class CDeviceCommand
 *   Provides common, shared services for classes that implement device support
 *   commands.
 */
class CDeviceCommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;    
public:
    // Canonicals:
    
    CDeviceCommand(
        CTCLInterpreter& interp, const char* command, CConfiguration& config
    );
    virtual ~CDeviceCommand() {}
    
    // The following implementations (operator(), configure and cget), are pretty much
    // the same between all command classes... we support an override however.
    

protected:
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual int create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) = 0;
    virtual int config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    virtual int cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    // Utilities for derived classes:
    
protected:
    int configure(
        CTCLInterpreter& interp, CReadoutModule*  pModule,
        std::vector<CTCLObject>& config, int firstPair = 3
    );
    std::string configMessage(
        std::string base, std::string key, std::string value,
        std::string errorMessage
    );
    virtual void Usage(
        CTCLInterpreter& interp, std::string msg,
        std::vector<CTCLObject> objv
    );

};


#endif
