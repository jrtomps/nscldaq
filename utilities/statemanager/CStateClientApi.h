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
# @file   CStateClientApi.h
# @brief  Class containing public interfaces for reactors to the state manager.
# @author <fox@nscl.msu.edu>
*/

#ifndef CSTATECLIENTAPI_H
#define CSTATECLIENTAPI_H

#include <CBufferQueue.h>
#include <string>
#include <stdexcept>
#include <Thread.h>


class CVarMgrApi;
class CVarMgrSubscriptions;
class CMutex;


/**
 ** @class CStateClientApi
 *    Defines an API for programs that react to state changes.
 *    Note that since I don't believe in friendship (C++) there
 *    will be methods that are public that are not intended
 *    to be called by the general public but only by classes that
 *    interact to maintain the state monitor, and tests.
 *    These will be indicated in this header.
 */
class CStateClientApi
{

public:
    typedef CBufferQueue<std::string> StateQueue;
private:
        // Thread that monitors state changes:
    
    class CMonitorThread : public Thread {
        private:
            CStateClientApi*      m_pApi;
            CVarMgrSubscriptions* m_pSubs;
            bool                  m_exit;
        public:
            CMonitorThread(
                std::string name,
                CStateClientApi* api,
                CVarMgrSubscriptions* pSubs
            );
            virtual ~CMonitorThread();
            
            virtual void run();
            void scheduleExit();
        private:
            void subscribe();
    };
    
private:
    CMutex*               m_guard;
    CVarMgrApi*           m_pApi;
    CVarMgrSubscriptions* m_pSubscriptions;
    StateQueue            m_StateChanges;
    std::string           m_lastState;           // Cache to not beat on server.
    bool                  m_standalone;
    std::string           m_programName;
    CMonitorThread*       m_pMonitor;
    // Canonicals:
    
public:
    CStateClientApi(const char* reqURI, const char* notURI, const char* programName);
    virtual ~CStateClientApi();
    
    // Queries on the program's variables:
    
public:
    std::string getState()  {return m_lastState;}
    void        setState(std::string newState);
    bool        isEnabled();
    bool        isStandalone() const {return m_standalone; }
    std::string title();
    int         runNumber();
    bool        recording();
    std::string outring();
    std::string inring();

    
    bool waitTransition(std::string& newState, int timeout = -1);
    
    // Errors throw this:
    
    class CException : public std::runtime_error {
    public:
        CException(std::string what) noexcept : runtime_error(what) {}
    };
    
    // Not intended for general consumption:
    
public:
    void postTransition(std::string newState);
    void updateStandalone(bool newValue);
    bool getStandalone();
    
    std::string getProgramDirectory();
    std::string getProgramVar(const char* varname);
    
    // Utilities:
    
private:
    void createApi(const char* uri);
    void createSubscriptions(const char* uri);
    void freeResources();
    
    

    
};


#endif