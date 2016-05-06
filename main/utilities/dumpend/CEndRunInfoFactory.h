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
# @file   CEndRunInfoFactory.h
# @brief  Define a factory that creates CEndRunInfo objects.
# @author <fox@nscl.msu.edu>
*/
#ifndef CENDRUNINFOFACTORY_H
#define CENDRUNINFOFACTORY_H

class CEndRunInfo;

/**
 * @class CEndRunInfoFactory
 *     Provides a factory class which knows how to create CEndRunInfo object
 *     given specific criteria.
 */
class CEndRunInfoFactory
{
public:
    typedef enum _DAQVersion {
        nscldaq11, nscldaq10
    } DAQVersion;
public:
    static CEndRunInfo* create(int fd);                      // Create on on an open file.
    static CEndRunInfo* create(DAQVersion version, int fd);  // Create given a version and fd
};
#endif

