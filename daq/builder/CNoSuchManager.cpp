/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

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
#include "CNoSuchManager.h"
#include "BuilderConstants.h"

/*!
   Construct an instance of the exception:
   @param manager     - The name of the manager being manipulated.
   @param pDoing      - Description of what was happening when that manager was
                        'manipulated'.
*/
CNoSuchManager::CNoSuchManager(std::string manager,
			       const char* pDoing) :
  CSourceException(pDoing, manager)
{}
/*!
  Machine readable error code.

  @return int
  @retval BuilderConstant::NO_SUCH_MANAGER
*/
int 
CNoSuchManager::ReasonCode() const
{
  return BuilderConstant::NO_SUCH_MANAGER;
}
/*!
  Human readable error reason

  @return std::string
*/

const char* 
CNoSuchManager::ReasonText() const
{
  m_ReasonText   = "An attempt was made to manpulate the nonexistent manager; ";
  m_ReasonText  += getManager();
  return m_ReasonText.c_str();

}
