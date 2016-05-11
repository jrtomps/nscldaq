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
# @file   CCumulativeChannel.cpp
# @brief  Implements the logic of cumulative (non-cleared) channels.
# @author <fox@nscl.msu.edu>
*/


#include "CCumulativeChannel.h"


/**
 * constructor
 *    clear the last value and wrap adjust
 */
CCumulativeChannel::CCumulativeChannel() :
    m_lastValue(0), m_WrapAdjust(0)
{}


/**
 * update
 *    Update the value from a new read of the scaler.
 *
 *  @param value - new value read from the scaler.
 *  @param width - Scaler width in bits.
 */
void
CCumulativeChannel::update(unsigned value, unsigned width)
{
    uint64_t mask = 1;
    uint64_t wrapvalue;
    wrapvalue     = mask << width;             // What a wrap means.
    mask          = wrapvalue - 1;             // mask of channel.
    
    uint64_t scaler = value;
    scaler          = scaler & mask;
    
    if (scaler < m_lastValue) {
        m_WrapAdjust += wrapvalue;
    }
    m_lastValue = scaler;
}
/**
 * uint64_t
 *    Return the scaler sum.
 *
 *  @return uint64_t
 */
CCumulativeChannel::operator uint64_t()
{
    return m_WrapAdjust + m_lastValue;
}