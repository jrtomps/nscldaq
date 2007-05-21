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

#include <config.h>
#include <spectrodaq.h>

using namespace std;

/*!
   Startup will store our args and then execute start:
*/
void
DAQThread::Startup(int args, char**argv)
{
  m_args  = args;
  m_pArgs = argv;
  start();
}

/*!
   Entry point.. marshall for operator().
*/
void 
DAQThread::run()
{
  operator()(m_args, m_pArgs);
}

/*!
   Exit a thread:
*/
void
DAQThread::Exit(int status)
{
  dshwrapthread_exit(static_cast<void*>(&status));
}

/*!
   Mimic the old daq scheduler class/object.
*/
void
DAQScheduler::Dispatch(DAQThread& pThread, int argc, char** argv)
{
  pThread.Startup(argc, argv);
}

DAQScheduler daq_dispatcher;

