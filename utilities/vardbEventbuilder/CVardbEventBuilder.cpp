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
# @file   CVardbEventBuilder.cpp
# @brief  Implements the API to the event builder description in variable database.
# @author <fox@nscl.msu.edu>
*/
#include "CVardbEventBuilder.h"
#include <CVarMgrApiFactory.h>
#include <CVarMgrApi.h>


/**
 * constructor
 *    Construct a new instance of this class.  Specifically, we need
 *    to use the variable manager api factory to create an instance
 *    of our api object.
 *
 *    @param uri - Uri that identifies the connection method to the database.
 */
CVardbEventBuilder::CVardbEventBuilder(const char* uri) :
    m_pApi(0)
{
    m_pApi = CVarMgrApiFactory::create(uri);   // Let exceptions propagate up.
}
/**
 * destructor
 *    Kill off the api object.
 */
CVardbEventBuilder::~CVardbEventBuilder()
{
    delete m_pApi;
}
