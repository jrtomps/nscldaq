#ifndef __CDAQMODULECOMMAND_H
#define __CDAQMODULECOMMAND_H

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

class CTCLInterpreter;
class CTCLObject;

class CConfiguration;
class CConfigurableObject;

/*!
  ABSTRACT BASE CLASS

   This class is a base class for commands that manipulate VM-USB modules.
   In most cases the logic for these commands is common.  We only need to 
   delegate to the concrete derived class (we are an ABC):
   - createObject creates and returns an actual object (dynamically created).
   - getType      returns the object type string under which the object will
                  be entered in the configuration database.

   We supply the following member functions:
   - virtual operator()  - Dispatches based on subcommand to one of the members below:
   - virtual create      - Creates a new modules and performs optional partial configuration.
                           To retain compatibility, this supports a two syntaxes. The first is:
                           command create name baseAddress, and generates a configure for
                           the -base attribute.  The second is of the form
                           command create name key value...
                           and simply uses the part of the command following the object name 
                           as configuration items.
  - virtual config       - Configures an existing object.
  - virtual cget         - returns the configuration list of an existing object.
  - virtual Usage        - Creates a usage string and sets it as the result code.

  The preceding members are also virtual so that module commands with special needs can
  override or extend them.
*/
class CDAQModuleCommand : public CTCLObjectProcessor // >>>ABSTRACT BASE CLASS<<<
{
protected:
  CConfiguration&  m_config;
public:
  CDAQModuleCommand(CTCLInterpreter& interp,
		 CConfiguration&  config,
		 std::string      name);
  virtual ~CDAQModuleCommand();

private:
  CDAQModuleCommand(const CDAQModuleCommand& rhs);
  CDAQModuleCommand& operator=(const CDAQModuleCommand& rhs);
  int operator==(const CDAQModuleCommand& rhs) const;
  int operator!=(const CDAQModuleCommand& rhs) const;


  // This class implements the following methods, although they can be
  // overridde in concrete sublcasses.

public:
  int operator()(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv);

protected:
  virtual int create(CTCLInterpreter& interp, 
                     std::vector<CTCLObject>& objv);
  virtual int config(CTCLInterpreter& interp,
                     std::vector<CTCLObject>& objv);
  virtual int cget(CTCLInterpreter& interp,
                   std::vector<CTCLObject>& objv);
  virtual void Usage(std::string msg, std::vector<CTCLObject>& objv);


  // The derived concrete classes must supply implementations for the following
  // methods:

protected:
  virtual CConfigurableObject* createObject() = 0;
  virtual std::string          getType()      = 0;

  // The following are utility method is also virtual so special requirements can
  // override it:

protected:
  virtual void addObjectToConfiguration(CConfiguration& config,
					std::string     name,
					std::string     type,
					CConfigurableObject* object);

  // Internal utility functions:

private:
  CConfigurableObject* findModule(std::string name);
  CConfigurableObject* findModuleOfMyType(std::string name);
  int Configure(CTCLInterpreter&         interp,
		std::vector<CTCLObject>& objv,
		CConfigurableObject*     pObject,
		unsigned                 startAt = 3);
};



#endif
