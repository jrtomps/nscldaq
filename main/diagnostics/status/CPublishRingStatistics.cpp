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
# @file   CPublishRingStatistics.cpp
# @brief  Implement class to publish ring buffer statistics.
# @author <fox@nscl.msu.edu>
*/

#include "CPublishRingStatistics.h"
#include "CStatusMessage.h"
#include <os.h>
#include <CRingMaster.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>



/**
 * constructor:
 *    Just salt away the socket for use when we publish.
 *
 *  @param socket - reference to the zmq::socket_t on which we'll send messages.
 *  @param appName - Application name sent in messages.
 */
CPublishRingStatistics::CPublishRingStatistics(
    zmq::socket_t& socket, std::string appName
) :
    m_pSocket(&socket),
    m_appName(appName)
{}

/**
 * destructor
 *   At this point our client is responsible for the disposition of the socket
 *   so this is a no-op:
 */
CPublishRingStatistics::~CPublishRingStatistics()
{}

/**
 * operator()
 *    Actually publish the data:
 *    - Obtain the ring buffer usage.
 *    - If necessary, create a CStatusDefinitions::RingStatistics object
 *    - If necessary, push messages for all the rings through that object.
 *  @note 'If necessary' above means that the number of rings in existence
 *        is non-zero.
 */
void
CPublishRingStatistics::operator()()
{
    CRingMaster rmaster;                        // Only want for the localhost.
    std::string usage = rmaster.requestUsage();
    std::vector<Usage> usageVector                   = usageTextToVector(usage);
    publish(usageVector);
    
}

/*---------------------------------------------------------------------------
 *  Private utilities:
 */

/**
 * usageTextToVector
 *    Convert the usage text from the ring master into a vector
 *    Usage structs.  ring master text is a Tcl list of lists
 *    where each sublist has:
 *    -  Name of the ringbuffer
 *    -  List containing statistics, which is as follows:
 *       * Buffer size.
 *       * Bytes avavilable
 *       * Number of consumers allowed
 *       * Producer PID (-1 if none).
 *       * max get space
 *       * min get space.
 *       * List of consumer pids, backlogs
 *       * List of statistics containing
 *         # producer status (puts and bytes).
 *         # For each consumer a triple of the pid, get count and bytes.
 *
 *   @param usageString - Usage string from the ringmaster.
 *   @return std::vector<Usage> - struct that contains the ring buffer statistics.
 */
std::vector<CPublishRingStatistics::Usage>
CPublishRingStatistics::usageTextToVector(std::string& usage)
{
    std::vector<Usage> result;
    
    // Convert the usage list into a Tcl object:
    
    CTCLInterpreter interp;
    CTCLObject      usageList;
    usageList.Bind(interp);
    usageList = usage;
    
    for (int i = 0; i < usageList.llength(); i++) {
        
        // Got an item.
        
        CTCLObject item;
        item.Bind(interp);
        item = usageList.lindex(i);
        
        Usage oneUsage = itemToUsage(interp, item);
        result.push_back(oneUsage);
    }
    
    return result;
}
/**
 * itemToUsage
 *    Take a single ring buffer item and turn it into a Usage struct.
 *
 *  @param interp - Tcl Interpreter to use to parse items.
 *  @param obj    - Single list item object.  See above
 *                  for contents.
 *  @return CPublishRingStatistics::Usage struct.
 */
CPublishRingStatistics::Usage
CPublishRingStatistics::itemToUsage(CTCLInterpreter& interp, CTCLObject& obj)
{
    Usage result;
    
    CTCLObject oRingName  = obj.lindex(0);
    CTCLObject oRingStats = obj.lindex(1);
    oRingName.Bind(interp);
    oRingStats.Bind(interp);
    
    result.s_ringName = std::string(oRingName);
    
    result.s_usage.s_bufferSpace = int(oRingStats.lindex(0));
    result.s_usage.s_putSpace    = int(oRingStats.lindex(1));
    result.s_usage.s_maxConsumers= int(oRingStats.lindex(2));
    result.s_usage.s_producer    = int(oRingStats.lindex(3));
    result.s_usage.s_maxGetSpace = int(oRingStats.lindex(4));
    result.s_usage.s_minGetSpace = int(oRingStats.lindex(5));
    
    CTCLObject oConsumers = oRingStats.lindex(6);
    oConsumers.Bind(interp);
    
    // Consumer backlogs:
    
    for (int i = 0; i < oConsumers.llength(); i++) {
        CTCLObject item = oConsumers.lindex(i);
        item.Bind(interp);
        CTCLObject oPid     = item.lindex(0);
        CTCLObject oBacklog = item.lindex(1);
        oPid.Bind(interp);
        oBacklog.Bind(interp);
        
        std::pair<pid_t, size_t> consumer;
        consumer.first = int(oPid);
        consumer.second = int(oBacklog);
        result.s_usage.s_consumers.push_back(consumer);
    }
    // Producer statistics:
    
    CTCLObject oPstats = oRingStats.lindex(7);     // Producer statistics.
    oPstats.Bind(interp);
    CTCLObject ops;
    ops.Bind(interp);
    CTCLObject bytes;
    bytes.Bind(interp);
    
    ops = oPstats.lindex(0);
    bytes = oPstats.lindex(1);
    result.s_usage.s_producerStats.s_pid = result.s_usage.s_producer;
    result.s_usage.s_producerStats.s_transfers = double(ops);
    result.s_usage.s_producerStats.s_bytes     = double(bytes);
    if (result.s_usage.s_producer != -1) {
        result.s_producerCommand =
            Os::getProcessCommand(result.s_usage.s_producer);
    }
    
    CTCLObject oCstats;                    // Consumer statistics
    oCstats.Bind(interp);
    oCstats = oRingStats.lindex(8);
    for (int i = 0; i < oCstats.llength(); i++) {
        CTCLObject item;
        item.Bind(interp);
        item = oCstats.lindex(i);
        
        CTCLObject oPid;
        oPid.Bind(interp);
        oPid = item.lindex(0);
        ops  = item.lindex(1);
        bytes = item.lindex(2);
        
        CRingBuffer::clientStatistics client;
        client.s_pid = int(oPid);
        client.s_transfers= double(ops);
        client.s_bytes = double(bytes);
        result.s_usage.s_consumerStats.push_back(client);
        if (client.s_pid != -1) {
            result.s_consumerCommands.push_back(
                Os::getProcessCommand(client.s_pid)
            );
        }
        
    }
    
    return result;
}
/**
 * publish
 *    Perform the actual publication.
 *    - Constructs the RingStatistics object,
 *    - Iterates over the ring information and sends message clumps for each ring.
 *
 * @param usage - The list of usages:
 */
void
CPublishRingStatistics::publish(std::vector<Usage>& usage)
{
    CStatusDefinitions::RingStatistics publisher(*m_pSocket, m_appName);
    
    for (int i = 0; i < usage.size(); i++) {
        Usage& item(usage[i]);
        
        publisher.startMessage(item.s_ringName);
        
        // Add producer information if there's a producer:
        
        if (item.s_usage.s_producer != -1) {
            publisher.addProducer(
                item.s_producerCommand,
                item.s_usage.s_producerStats.s_transfers,
                item.s_usage.s_producerStats.s_bytes,
                item.s_usage.s_producer
            );
        }
        // add any and all consumers:
        
        for (int c = 0; c < item.s_consumerCommands.size(); c++) {
            CRingBuffer::clientStatistics&
                stats(item.s_usage.s_consumerStats[c]);
            publisher.addConsumer(
                item.s_consumerCommands[c], stats.s_transfers, stats.s_bytes,
                item.s_usage.s_consumers[c].second,             // Backlog
                stats.s_pid
            );
                                  
        }
        publisher.endMessage();                   // Send the message.
    }
}