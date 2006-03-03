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

//////////////////////////CRunVariable.h file//////////////////////////////////

#ifndef __CRUNVARIABLE_H  
#define __CRUNVARIABLE_H

#ifndef __CTCLVARIABLE_H
#include <TCLVariable.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __TCLINTERPRETER_H
#include <TCLInterpreter.h>
#endif


/*!

   Encapsulates a run variable.  A run variable is one which is periodically
   written to a run variable buffer during an active run.
   
 */		


class CRunVariable : public CTCLVariable     
{ 
private:

public:
	// Constructors, destructors and other cannonical operations: 

    CRunVariable (CTCLInterpreter* pInterp,
		  STD(string)&          rName); //!< Constructor.
    CRunVariable(const CRunVariable& rhs); //!< Copy constructor.
     ~ CRunVariable ( ) { } //!< Destructor.
    CRunVariable& operator= (const CRunVariable& rhs); //!< Assignment
    int         operator==(const CRunVariable& rhs) const; //!< Comparison for equality.
    int         operator!=(const CRunVariable& rhs) const {
       return !(operator==(rhs));
    }


	// Class operations:

public:
     STD(string) FormatForBuffer (int nMaxchars=-1)  ; //!< Stringify name:value...

 
};

#endif

