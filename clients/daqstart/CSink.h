// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

/*!
  \class CSink

  Abstract base class of the sink class hierarchy.

*/

#ifndef __CSINK_H  //Required for current class
#define __CSINK_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class CSink      
{
private:
  
    string m_sCommand;  //!< The command string to put in messages.  
   
protected:
	// Protected member data.
 
public:
	// Public member data.

public:
  
  CSink (string sLine);	          //!< Constructor.
  virtual  ~ CSink ( );                 //!< Destructor.
  CSink (const CSink& rSource );        //!< Copy construction.
  CSink& operator= (const CSink& rhs);  //!< Assignment.
  int operator== (const CSink& rhs) const; //!< == comparison.
  int operator!= (const CSink& rhs) const {  //!< != comparison.
    return !(operator==(rhs));
  }


public:
  virtual CSink*  clone() = 0;	  //!< Return dup of this .
  virtual   bool Log (const string& Message)   = 0 ; 
  virtual   string FormatLine (string sMessage)   ; 


};

#endif
