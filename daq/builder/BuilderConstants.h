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
#ifndef __BUILDERCONSTANTS_H
#define __BUILDERCONSTANTS_H


class CBuilderConstant {
public:
  typedef enum _EventType_ {
    PHYSICS,
    SCALER,
    BEGIN,
    END,
    PAUSE,
    RESUME,
    PACKETDOCS,
    VARIABLES,
    TRIGGERCOUNT,
    NOEVENT
  } EventType;
};

typedef enum _ExceptionReasonCodes {
  INVALID_OBJECT_TYPE_EXCEPTION,
  INVALID_CONFIG_PARAM_NAME,
  INVALID_CONFIG_PARAM_VALUE,
  INVALID_STATE,
  NO_SUCH_MANAGER
} ExceptionReasonCode;

#endif
