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
#ifndef __ASSEMBLERAPPLICATION_H
#define __ASSEMBLERAPPLICATION_H

#ifndef __TCLAPPLICATION_H
#include <TCLApplication.h>
#endif



/*!
   This is the application object that the TCL framework uses
   to start up the application.
*/
class AssemblerApplication : public CTCLApplication
{
public:
    virtual int operator()();                    // Define/create the commands.
};


#endif