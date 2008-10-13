#ifndef __CREADOUTMAIN_H
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


#ifndef __TCLAPPLICATION_H
#include <TCLApplication.h>
#endif


class CReadoutMain : public CTCLApplication
{
public:
  virtual int operator()();
protected:
  virtual void SetupRunVariables();
  virtual void SetupStateVariables();
  virtual void SetupReadout() = 0;
  virtual void SetupScalers();

};




#endif
