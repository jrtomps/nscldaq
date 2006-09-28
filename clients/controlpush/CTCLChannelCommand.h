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

#ifndef __CTCLCHANNELCOMMAND_H
#define __CTCLCHANNELCOMMAND_H

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


class CTCLInterpreter;
class CTCLObject;
class CChannel;
class CTCLVariable;

/*!
   This class implements an epics channel command  Epics channel commands
   are created for each channel the user is interested in.
   These commands have names like the channel/field name and form an
   ensemble with the following subcommands:
   - get                   - Gets the current value of the channel.
   - set value  ?datatype? - Sets the value of the channel as shown.
                             The optional data type is one of 
                           - string to set a string value.
			   - int to set an integer value.
			   - real to set a real (double) value.
   - updatetime            - returns the last time the channel was updated,
                             in form suitable for use with [clock format].
   - delete                - Destroys this command and underlying data etc.
   - link tclvarname       - Links the channel to a Tcl variable.
   - unlink                - Removes any variable linkage.
*/
class CTCLChannelCommand : public CTCLObjectProcessor 
{
private:
  CChannel*      m_pChannel;
  CTCLVariable*  m_pLinkedVar;

  // Canonicals:
public:
  CTCLChannelCommand(CTCLInterpreter& interp,
		     STD(string)      name);
  virtual ~CTCLChannelCommand();
private:
  CTCLChannelCommand(const CTCLChannelCommand& rhs);
  CTCLChannelCommand& operator=(const CTCLChannelCommand& rhs);
  int operator==(const CTCLChannelCommand& rhs) const;
  int operator!=(const CTCLChannelCommand& rhs) const;
public:

  virtual int operator()(CTCLInterpreter& interp,
			 STD(vector)<CTCLObject>& objv);

  void UpdateLinkedVariable();
  // Utilties:

private:
  int Get(CTCLInterpreter& interp);
  int Set(CTCLInterpreter& interp, STD(vector)<CTCLObject>& objv);
  int Updatetime(CTCLInterpreter& interp);
  int Delete(CTCLInterpreter& interp);
  int Link(CTCLInterpreter& interp, STD(vector)<CTCLObject>& objv);
  //  int Unlink(CTCLInterpreter& interp);
  STD(string) Usage();


};

#endif
