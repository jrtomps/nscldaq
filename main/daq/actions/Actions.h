
#ifndef ACTIONS_H
#define ACTIONS_H

#include <iostream>
#include <string>
#include <time.h>
#include <map>
#include <sstream>

/**! The C++ functions for the actions package */

/**
 * @note In 12.0, the implementation should be separated from the
 *       header.  That allows the implementation to get complicated while
 *       not complicating the header.  Since I'm doing throttling in the
 *       11.0 branch this is going to get complex...but we have Makefiles out there
 *       we can't affect so here goes...
 */

namespace Actions 
{
    /* Private code - do not call these methods (hmm reimplementation
     * should make this a class with static methods so privacy can be enforced) */
    
    typedef struct _lastMessageInfo {
        std::string s_message;
        time_t      s_firstTime;
        size_t      s_count;                   // Additional consecutive counts.
    } lastMessageInfo, *pLastMessageInfo;
    
    std::map<std::string, lastMessageInfo> m_messageHistory;
    /**
     * buildMessage
     *    Build a message.  This is called when we already know we want a message
     *    emitted (it's been checked and found not to be a repeat or found to
     *    be out of time).
     *
     *    @param type - type of message.
     *    @param body - message body.
     *    @return std::string the message string in the form "type bodysize body\n"
    */
    std::string buildMessage(std::string type, std::string body) {
        std::stringstream msg;
        msg << type << " " << body.size() << " " << body << std::endl;
        return msg.str();
    }
    /**
     * buildMessage
     *    overloaded version of the prior method that has a count  The body
     *    is transformed to "n occurences of 'oldbody'" and then the prior
     *    buildMessage is used to build the result.
     * @param type - message type.
     * @param count - Number of repetitions
     * @param body  - Body that was repeated.
     * @return std::string - message to output.
     */
    std::string buildMessage(std::string type, size_t count, std::string body)
    {
        std::stringstream newBody;
        newBody << count << " occurences of '" << body << "'";
        return buildMessage(type, newBody.str());
    }
    /**
     * flushRepeatedMessages
     *   Sometimes its necessary to output messages prior to formatting the
     *   new message (e.g. when a different message body arrives).
     *   This outputs the message implied by the last message struct and type
     *
     *   @param type - type of message.
     *   @param msg  - Refers to a lastMessageInfo struct that describes the
     *                 message to output.
     */
    void
    flushRepeatedMessages(std::string type, const lastMessageInfo& msg)
    {
        std::cerr << buildMessage(type, msg.s_count, msg.s_message);
    }
    /**
     * Construct the message given the type and body and implicitly the history.
     * returns "" if nothing to output at this time...note a 'Big Flaw'  Suppose
     * there are a stream of messages that are all identical and then they stop.
     * In this implementation it's quite possible that the last set of
     * 'n occcurences of "this message"' won't go out until the next message
     * and that could be some time much later in the future.  The
     * 'right' way to solve this is, once more, to buld a class and have a thread
     * associated with that class that periodically wakes up and flushes old
     * messages.  Too much work to get right  in the next couple of days.
     *
     * @param - type of message.
     * @param - body - body of message.
     * @return std::string - if not empty the string should be outputted otherwise
     *                       not.
     */
    std::string constructMessage(std::string type, std::string body) {
        time_t now = time(NULL);
        
        // If the message type has no map entry fabricate one  --
        // emit the message as is.
        
        if (! m_messageHistory.count(type)) {
            lastMessageInfo thisMessage = {body, now, 0};
            m_messageHistory[type] = thisMessage;
            return buildMessage(type, body);
        } else {
            // If the time of the current entry differs from now flush it out.
            
            lastMessageInfo lastMessage = m_messageHistory[type];
            if ((now != lastMessage.s_firstTime)   &&
                (body == lastMessage.s_message)) {
                // update the record and return the duplicate message thingy.
                
                if (lastMessage.s_count > 0) {
                    flushRepeatedMessages(type, lastMessage);
                }
                
                lastMessage.s_count = 0;
                lastMessage.s_firstTime = now;
                m_messageHistory[type] = lastMessage;
                
                return buildMessage(type,  body);
            } else if (lastMessage.s_message != body) {
                // flush the last message, construct new last Message struct
                // with the current message and return the buit message with
                // the current message:
                
                if (lastMessage.s_count > 0) {
                    flushRepeatedMessages(type, lastMessage);
                }
                lastMessage.s_message    = body;
                lastMessage.s_count      = 0;
                lastMessage.s_firstTime  = now;
                m_messageHistory[type]   = lastMessage;
                return buildMessage(type, body);
            } else {
                // If here, the messages is the same and the time is the same...
                // just update the count. Return an empty message signallilng
                // no output needed at this time:
                
                lastMessage.s_count++;
                m_messageHistory[type] = lastMessage;
                return "";
            }
        }
    }
    
    void sendMessage(std::string type, std::string body) {
        std::string message = constructMessage(type, body);
        if (message != "") {
            std::cerr <<  message << std::flush;
        }
    }
    
    /* Public interface - only call functions below this line:  */
    
    void Error (std::string message ) {
        sendMessage("ERRMSG", message);
        
    }  

    void Log (std::string message ) {
        sendMessage("LOGMSG", message);
    }  

    void Warning (std::string message ) {
        sendMessage("WRNMSG", message);
    }  


    void Output (std::string message ) {
        sendMessage("OUTPUT", message);
    }  

    void Debug (std::string message ) {
        sendMessage("DBGMSG", message);

    }  
    void TCLCommand (std::string message ) {
        std::cerr << "TCLCMD " << message.size() << " " 
                  << message << "\n" << std::flush;
    }  

    void BeginRun () {
        TCLCommand ( "begin" );
    }

    void PauseRun () {
        TCLCommand ( "pause" );
    }

    void ResumeRun () {
        TCLCommand ( "resume" );
    }

    void EndRun (bool propagate=true) {
        if (propagate) {
          TCLCommand ( "end" );
        } else {
          TCLCommand ( "local_end" );
        }
    }
}

#endif
