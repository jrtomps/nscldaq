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
#include <list>

#include <zmq.hpp>
#include <ctime>

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
    

        // This describes the structure of the message header.
        
        struct Header {
            uint32_t s_type;
            uint32_t s_severity;
            char     s_application[32];
            char     s_source[128];
        };
        
        // These two structures are the two body parts for ring statistics:
        
        struct RingStatIdentification {
            uint64_t s_tod;
            char     s_ringName[];                // Variable length.
        };
        struct RingStatClient {                  // Not same order as wiki but
            uint64_t  s_operations;              // uses natural alignments.
            uint64_t  s_bytes;
            uint32_t  s_isProducer;
            char      s_command[];               // Now this can be variable length.
        };
        // Structs that define message parts for ReadoutStatistics:
        
        struct ReadoutStatRunInfo {
            uint64_t s_startTime;
            uint32_t s_runNumber;
            char     s_title[80];
        };
        struct ReadoutStatCounters {
            uint64_t   s_tod;
            uint64_t   s_elapsedTime;
            uint64_t   s_triggers;
            uint64_t   s_events;
            uint64_t   s_bytes;
        };
        
        // Log message structure:
        
        struct LogMessageBody {
            uint64_t   s_tod;
            char       s_message[];
        };
        
        // State change message parts:
        
        struct StateChangeBody {
            uint64_t s_tod;
            char     s_leaving[32];
            char     s_entering[32];
        };
    class MessageTypes {
    public:
        static const std::uint32_t RING_STATISTICS;
        static const std::uint32_t EVENT_BUILDER_STATISTICS;
        static const std::uint32_t READOUT_STATISTICS;
        static const std::uint32_t LOG_MESSAGE;
        static const std::uint32_t STATE_CHANGE;
        
        // keep this updated as new types are added:
        
        static const std::uint32_t FIRST_FREE_TYPE;
        
        //
        
        static const std::uint32_t FIRST_USER_TYPE;
    };
    /**
     * @class CStatusDefinitions::SeverityLevels
     *    Defines the message severity levels that can be put in the  message
     *    severity field.
     */
    public:
    class SeverityLevels {
    public:
        static const std::uint32_t DEBUG;
        static const std::uint32_t INFO;
        static const std::uint32_t WARNING;
        static const std::uint32_t SEVERE;
        static const std::uint32_t DEFECT;
    };
    /**
     * @class  CStatusDefinitions::RingStatistics
     *   This class is responsible for
     *   - Formatting ring status messages
     *   - sending the formatted message to the XSUB server.
     */
    public:
    class RingStatistics {
    private:
        zmq::socket_t&            m_socket;
        std::string               m_applicationName;
        bool                      m_msgOpen;
        std::string               m_ringName;
        RingStatClient*           m_producer;
        std::list<RingStatClient*> m_consumers;
        
    public:
        RingStatistics(zmq::socket_t& socket, std::string app="RingStatDaemon");
        virtual ~RingStatistics();
        
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
        RingStatClient* makeClient(
            std::vector<std::string> command, uint64_t ops, uint64_t bytes,
            bool producer = false
        );
        void freeStorage();
        size_t sizeClient(RingStatClient* pClient);
        
    };
    /**
     * @class EventBuilderStatistics
     *    Responsible for
     *    - Formatting event builder statistics messages.
     *    - Sending these message to the XSUB server.
     */
    public:
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
    public:
    class ReadoutStatistics {
        zmq::socket_t&  m_socket;
        std::string     m_appName;
        std::time_t     m_runStartTime;
        bool            m_haveOpenRun;
    public:
        ReadoutStatistics(zmq::socket_t& socket, std::string app = "Readout");
        virtual ~ReadoutStatistics();
        
        
        void beginRun(std::uint32_t runNumber, std::string title);
        void emitStatistics(
            std::uint64_t triggers, std::uint64_t events, std::uint64_t bytes
        );
    };
    /**
     * @class LogMessage
     *    Responsible for emitting log messages for the client.
     */
    public:
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
    public:
    class StateChange
    {
    public:
        StateChange(zmq::socket_t& socket, std::string app);
        ~StateChange();
        
        void logChange(std::string leaving, std::string entering);
    };
    // Utility methods for building messages:
    
    private:
        static Header formatHeader(
            uint32_t type, uint32_t severity, const char* appName
        );

};
#endif