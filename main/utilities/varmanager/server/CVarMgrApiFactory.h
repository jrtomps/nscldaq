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
# @file   CVarMgrApiFactory.h
# @brief  Defines the factory class for the public Variable manager API.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRAPIFACTORY_H
#define CVARMGRAPIFACTORY_H

#include <string>

class CVarMgrApi;
class URI;
/**
 * @class CVarMgrApiFactory
 *     Class that can manufacture API Objects given a URI that points to them.
 *     The URI's passed to the factory method are of one of the following forms:
 *     - file:///path/to/database/file - Builds a CVarMgrFileApi that operates on
 *              /path/to/database/file
 *     - tcp://hostname:port           - Builds a CVarMgrServerApi that connects
 *               to the already running database server on hostname whose REQ/REP
 *               port is given by the numeric port part of the URI.
 *     - tcp://hostname/service-name - Same as above, however the numeric port
 *               is gotten by looking it up with the port manager of hostname.
 *     - tcp://hostname  - Same as above, however the default service name is used
 *               to look up the REQ/REP port.
 */
class CVarMgrApiFactory
{
public:
    static CVarMgrApi* create(std::string uri);
    static CVarMgrApi* createFileApi(std::string path);
    static CVarMgrApi* createServerApi(std::string host, int port);
    static CVarMgrApi* createServerApi(std::string host, std::string service = std::string("vardb-request"));
    
};



#endif