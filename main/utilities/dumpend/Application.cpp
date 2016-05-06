/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   Application.cpp
# @brief  Implement the application's main logic.
# @author <fox@nscl.msu.edu>
*/

#include "Application.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "CEndRunInfo.h"
#include "CEndRunInfoFactory.h"

#include <iostream>
#include <exception>
#include <string>
#include <errno.h>
#include <string.h>
#include <time.h>

/**
 * constructor
 *   - save the parsed options away until we run.
 *
 *  @param ags - arguments parsed by gengetopt.
 */

Application::Application(gengetopt_args_info& args) : m_Args(args)
{}

/**
 * destructor
 */
Application::~Application()
{}

/**
 * operator()
 *    The action of the application.
 *    Process each of the files in accordance with the values of
 *    the options.  Output is to cout.
 */
void
Application::operator()()
{
    for (int i = 0; i < m_Args.inputs_num; i++) {
        const char* fileName = m_Args.inputs[i];
        processFile(fileName);
    }
}
/*--------------------------------------------------------------------------
 * Private utilities
 */

/**
 * processFile
 *    Process one file on the command line:
 *    -  Attempt to open it.  If that fails, report why and return.
 *    -  If the file opens properly create an end info object on it as directed
 *       by the daqversion switch.
 *    - Dump the end run information to cout.
 */
void
Application::processFile(const char* name)
{
    // open the file (if possible)
    
    int fd = open(name, O_RDONLY);
    if (fd < 0) {
        int e = errno;
        std::cerr << "dumpend: Unable to open: " << name << " : " << strerror(e) << std::endl;
        return;
    }
    //  How we create the end run info object depends on the value of the --daqversion switch:
    
    CEndRunInfo* pEndRun(0);
    if (m_Args.daqversion_arg == daqversion__NULL) {
        pEndRun = CEndRunInfoFactory::create(fd);    // Figure out from contents.
    } else {
        CEndRunInfoFactory::DAQVersion v;
        switch (m_Args.daqversion_arg) {
            
            case daqversion_arg_11:
                v = CEndRunInfoFactory::nscldaq11;
                break;
            case daqversion_arg_10:
                v = CEndRunInfoFactory::nscldaq10;
                break;
        }
        pEndRun = CEndRunInfoFactory::create(v, fd);
    }
    
    // Close the file, dump the info and delete the end run info object:
    
    close(fd);
    dumpEndRunInfo(name, *pEndRun);
    delete pEndRun;
    
}

/**
 *  dumpEndRunInfo
 *     Dump end run info to stdout.
 *     @param name   - name of the run file.
 *     @param endRun - Reference to the end run information object.
 */
void
Application::dumpEndRunInfo(const char* name, CEndRunInfo& endRun)
{
    std::cout << "------------------ " << name << " ----------------------\n";
    std::cout << "Has " << endRun.numEnds() << " end run records\n";
    for (int i = 0; i < endRun.numEnds(); i++) {
        std::cout << "End run record # " << i << std::endl;
        if (endRun.hasBodyHeader(i)) {
            dumpBodyHeader(i, endRun);
        }
        dumpBody(i, endRun);
    }
}
/**
 * dumpBodyHeader
 *    Dumps info from an end run body header.
 *
 *    @param i - selects the end run record.
 *    @param e - References the end run object.
 */
void
Application::dumpBodyHeader(int i, CEndRunInfo& e)
{
    std::cout << "Has a body header:\n";
    std::cout << "     Event timestamp: " << e.getEventTimestamp(i) << std::endl;
    std::cout << "     Source Id      : " << e.getSourceId(i) << std::endl;
    std::cout << "     Barrier Type   : " << e.getBarrierType() << std::endl;

}
/**
 * dumpBody
 *    Dump info from the end run body:
 *    @param i - selects the end run record.
 *    @param e - The end run information object.
 */
void
Application::dumpBody(int i, CEndRunInfo& e)
{
    time_t tod = e.getTod(i);
    std::cout << "Body: \n";
    std::cout << "      Run                : " << e.getRunNumber(i) << std::endl;
    std::cout << "      Seconds run lasted : " << e.getElapsedTime(i) << std::endl;
    std::cout << "      Run Title          : " << e.getTitle(i)  << std::endl;
    std::cout << "      Run Ended at       : " << ctime(&tod) << std::endl;
}