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

#ifndef __CPARAMMAPCOMMAND_H

#ifndef __TCLOBJECTCOMMAND_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


// Forward class definitions:

class CTCLObject;
class CTCLInterpreter;

/*!
   This class implements the Tcl paramMap command extension.
   This class is used to create and maintain a mapping between
   digitizer slots and parameter names in SpecTcl.  This is used
   by the unpacker to know where to put parameters it has unpacked.
   
   The command form is:
  
\verbatim
   paramMap slot channel name
\endverbatim

   Where:
   - slot    - is the adc slot number
   - channel - is the channel within the slot.
   - name    - is the name of the parameter at that slot.

   Note that at the time the paramMap command is issued, the parameter
   name must have already been defined.  In this way we are able to put
   parameter numbers in the mapping so that at unpack time we don't have
   to manipulate pesky strings.

*/
class CParamMapCommand : public CTCLObjectProcessor
{
  // Exported data types:

public:
  struct AdcMapping
  {
    int map[32];

    AdcMapping() { for(int i=0; i < 32; i++) map[i] = -1;}
    AdcMapping(const AdcMapping& rhs) {
      for(int i =0; i < 32; i++) map[i] = rhs.map[i];
    }
    AdcMapping& operator=(const AdcMapping& rhs) {
      for (int i =0; i < 32; i++) map[i] = rhs.map[i];
    }
    int& operator[](int i) { return map[i]; }

  };
  typedef STD(vector)<AdcMapping> ParameterMap;
private:
  static ParameterMap   m_theMap;

public:
  // Constructors and (disallowed) cannonicals:

  CParamMapCommand(CTCLInterpreter& interp);
  virtual ~CParamMapCommand();
private:
  CParamMapCommand(const CParamMapCommand& rhs);
  CParamMapCommand& operator=(const CParamMapCommand& rhs);
  int operator==(const CParamMapCommand& rhs) const;
  int operator!=(const CParamMapCommand& rhs) const;

public:
  static  const ParameterMap& getMap();

  // Virtual function overrides.
protected:
  int operator()(CTCLInterpreter& interp,
		 STD(vector)<CTCLObject>& objv);

  // utilities:

private:
  void createEntry(unsigned slot);
  static STD(string) Usage();
  static void setResult(CTCLInterpreter& interp, STD(string) result);
};

#endif
