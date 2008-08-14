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

#ifndef __CMARKERCOMMAND_H
#define __CMARKERCOMMAND_H

#ifndef __CMODULECOMMAND_H
#include <CModuleCommand.h>
#endif


/*!
   This class creates and configure marker stack entries.  A marker stack
   entry adds a literal 16 bit word to the output buffer for an event.

   The command supports the usual syntaxes for module generating commands:

\verbatim
  marker create name value
  marker config name -value new_value
  marker cget   name

\endverbatim

  As you can see, the only configuration option supported is

  - -value   Sets a new value for the marker.

  Requiring the value on the creation command line is how we ensure that
the value is mandatory
*/

class CMarkerCommand : public CModuleCommand
{

public:
  CMarkerCommand(CTCLInterpreter&     interp,
		 CConfiguration&      config,
		 std::string          commandName = std::string("marker"));
  virtual ~CMarkerCommand();

private:
  CMarkerCommand(const CMarkerCommand& rhs);
  CMarkerCommand& operator=(const CMarkerCommand& rhs);
  int operator==(const CMarkerCommand& rhs) const;
  int operator!=(const CMarkerCommand& rhs) const;
public:

  // The derived concrete classes must supply implementations for the following
  // methods:

protected:
  virtual CConfigurableObject* createObject();
  virtual std::string          getType();

};

#endif
