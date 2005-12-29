/*
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#
*/

//  Create a counting data source.

// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//    mailto:fox@nscl.msu.edu
//
//////////////////////////////////////////////////////////////////////////

static char* Copyright =
"(c) Copyright NSCL All rights reserved 1999 ReadoutMain.cpp\n";

//
// Include files:
//
#include <config.h>
#include <iostream>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <spectrodaq.h>


//
//   Global Variables:
//


extern int UsingVME;
void* pCamac;

class DAQBuff : public DAQROCNode {
  int operator()(int argc, char** argv);

};

int
DAQBuff::operator()(int argc, char**argv)
{
    int count = atoi(argv[1]);

    sleep(1);
    while (1) {
        DAQWordBuffer* buf = new DAQWordBuffer(count);
        buf->SetTag(3);
        for (int i =0; i < count; i++) {
            (*buf)[i] = i;
        }
        buf->Route();
        delete buf;
    }
}
DAQBuff mydaq;

