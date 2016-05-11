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
# @file   CCumulativeChannel.h
# @brief  Cumulative scaler channel
# @author <fox@nscl.msu.edu>
*/

#ifndef CCUMULATIVECHANNEL_H
#define CCUMULATIVECHANNEL_H

#include "CChannel.h"
#include <stdint.h>

/**
 * @class CCumulativeChannel
 *
 *  Cumulative scaler channels are those that are not cleared.
 *  To obtain the correct sum for these channels it is necessary to notice
 *  the number of times the channel 'wraps' - that is the number of times
 *  the channel appears to have a value smaller than its previous value.
 *
 *  Therefore we keep
 *    -   A last value
 *    -   A wrap adjustment.
 *
 *  @note There's tacit assumption that the channel has a constant bit width
 *        which is not a unreasonable.
 */
class CCumulativeChannel : public CChannel
{
private:
    uint64_t m_lastValue;
    uint64_t m_WrapAdjust;
public:
    CCumulativeChannel();
    
    void update(unsigned value, unsigned width=32);
    operator uint64_t();
};

#endif