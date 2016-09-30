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
# @file   CStatusMessage.h
# @brief  Messages and clients for status messages.
# @author <fox@nscl.msu.edu>
*/

#ifndef CSTATUSMESSAGE_H
#define CSTATUSMESSAGE_H
#include <cstdint>
#include <string>
#include <vector>
#include <zmq.hpp>

/**
 *  @class CStatusDefinitions
 *      Encapsulates the message types, severities and message structures
 *      common to the status aggregation system.
 */
class CStatusDefinitions
{
public:
    /**
     * @class CStatusDefinitions::MessageTypes
     *    Message type definitions.  These are legal values for the message
     *    type field of status messages.
     */
    class MessageTypes {
    public:
        static const std::uint32_t RING_STATISTICS          = 0x00000000;
        static const std::uint32_t EVENT_BUILDER_STATISTICS = 0x00000001;
        static const std::uint32_t READOUT_STATISTICS       = 0x00000002;
        static const std::uint32_t LOG_MESSAGE              = 0x00000003;
        static const std::uint32_t STATE_CHANGE             = 0x00000004;
        
        // keep this updated as new types are added:
        
        static const std::uint32_t FIRST_FREE_TYPE          = 0x00000005;
        
        //
        
        static const std::uint32_t FIRST_USER_TYPE          = 0x80000000;
    };
    /**
     * @class CStatusDefinitions::SeverityLevels
     *    Defines the message severity levels that can be put in the  message
     *    severity field.
     */
    class SeverityLevels {
    public:
        static const std::uint32_t DEBUG   = 0x00000000;
        static const std::uint32_t INFO    = 0x00000001;
        static const std::uint32_t WARNING = 0x00000002;
        static const std::uint32_t SEVERE  = 0x00000003;
        static const std::uint32_t DEFECT  = 0x00000004;
    };
    /**
     * @class  CStatusDefinitions::RingStatistics
     *   This class is responsible for
     *   - Formatting ring status messages
     *   - sending the formatted message to the XSUB server.
     */
    class RingStatistics {
    public:
        RingStatistics(zmq::socket_t& socket, std::string app="RingStatDaemon");
        ~RingStatistics();
        
        void startMessage(std::string ring);
        void addProducer(
            std::vector<std::string> command, std::uint64_t ops,
            std::uint64_t bytes
        );
        void addConsumer(
            std::vector<std::string> command, std::uint64_t ops,
            std::uint64_t bytes
        );
        void endMessage();
    private:
        void push_back(
            bool isProducer, std::vector<std::string> command,
            std::uint64_t ops, std::uint64_t bytes
        );
        
    };
    /**
     * @class EventBuilderStatistics
     *    Responsible for
     *    - Formatting event builder statistics messages.
     *    - Sending these message to the XSUB server.
     */
    class EventBuilderStatistics
    {
    public:
        EventBuilderStatistics(zmq::socket_t& socket, std::string app="NSCLEventBuilder");
        ~EventBuilderStatistics();
        
        // The format of this message is not yet known and  therefore the
        // methods needed to fill it in are also not yet known.
    };
    /**
     * @class ReadoutStatistics
     *    Responsible for
     *    - Formatting readout statistics messages.
     *    - Sending those messages on to the XSUB server.
     */
    class ReadoutStatistics {
    public:
        ReadoutStatistics(zmq::socket_t& socket, std::string app = "Readout");
        ~ReadoutStatistics();
        
        
        void beginRun(std::uint32_t runNumber, std::string title);
        void emitStatistics(
            std::uint64_t triggers, std::uint64_t events, std::uint64_t bytes
        );
    };
    /**
     * @class LogMessage
     *    Responsible for emitting log messages for the client.
     */
    class LogMessage {
    public:
        LogMessage(zmq::socket_t& socket, std::string app);
        ~LogMessage();
        
        void Log(SeverityLevels sev, std::string message);
    };
    /**
     * @class StateChanges
     *    Resposible for emitting state change messages for a client.
     */
    class StateChange
    {
    public:
        StateChange(zmq::socket_t& socket, std::string app);
        ~StateChange();
        
        void logChange(std::string leaving, std::string entering);
    };
    
    
};
#endif