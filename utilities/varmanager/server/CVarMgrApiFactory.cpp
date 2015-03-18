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
# @file   CVarMgrApiFactory.cpp
# @brief  Implements the database API factory.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrApiFactory.h"
#include <URL.h>
#include "CVarMgrFileApi.h"
#include "CVarMgrServerApi.h"

CVarMgrApi* CVarMgrApiFactory::create(std::string uri){ return 0; }
/**
 * createFileApi
 *    Create an api object for direct access to a database file:
 * @param[in] path - Path to the database file (must have already been initialized).
 */
CVarMgrApi*
CVarMgrApiFactory::createFileApi(std::string path)
{
    return new CVarMgrFileApi(path.c_str());
}
CVarMgrApi* CVarMgrApiFactory::createServerApi(std::string host, int port){ return 0; }
CVarMgrApi* CVarMgrApiFactory::createServerApi(std::string host, std::string service){ return 0; }
CVarMgrApi* CVarMgrApiFactory::createServerApi(std::string host){ return 0; }
