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
# @file   CIncrementalChannel.h
# @brief  Manage the sum of an incremental scaler channel.
# @author <fox@nscl.msu.edu>
*/

#ifndef CINCREMENTALCHANNEL_H
#define CINCREMENTALCHANNEL_H

#include "CChannel.h"
#include <stdint.h>

/**
 * @class CIncrementalChannel
 *     Encapsulates the sum of an incremental scaler channel.
 *     for this type of channel, the update is a mask and add operation.
 */
class CIncrementalChannel : public CChannel
{
    uint64_t m_sum;
public:
    CIncrementalChannel();
    
    void update(unsigned value, unsigned width=32);
    operator uint64_t();
};

#endif
