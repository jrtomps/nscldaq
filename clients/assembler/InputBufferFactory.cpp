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
#include "InputBufferFactory.h"
#include "InputBuffer.h"
#include "ScalerInputBuffer.h"
#include "StringListInputBuffer.h"
#include "StateTransitionInputBuffer.h"
#include "PhysicsInputBuffer.h"
#include <buftypes.h>
#include <unistd.h>

/*!
 *   The creational method.  At this time wwe don't know the byte ordering.
 * we're going to make a simplifying assumption:  There are not more than
 * 256 buffer types so we can compare the buffer type to itself or it's
 * byte swapped self.
 */
InputBuffer*
InputBufferFactory::create(void* pBuffer)
{
	// wrap the buffer type on top of itself and then select only
	// the bottom bits:
	
	uint16_t* pB = reinterpret_cast<uint16_t>(pBuffer);
	uint16_t type= pB[1];
	uint16_t swappedType;
	swab(&type, &swappedType, sizeof(uint16_t));
	type = (type | swappedType) & 0xff;
	
	InputBuffer* pResult(0);
	
	switch (type) {
	case DATABF:
		pResult = new PhysicsInputBuffer(pBuffer);
		break;
	case SCALERBF:
	case SNAPSCBF:
		pResult = new ScalerInputBuffer(pBuffer);
		break;
	case STATEVARBF:
	case RUNVARBF:
	case PKTDOCBF:
		pResult = new StringListInputBuffer(pBuffer);
		break;
	case BEGRUNBF:
	case ENDRUNBF:
	case PAUSEBF:
	case RESUMEBF:
		pResult = new StateTransitionInputBuffer(pBuffer);
		break;
	default:
		break;
	}

	return pFragment;
}