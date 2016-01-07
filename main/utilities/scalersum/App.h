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
# @file   App.h
# @brief  Application driver code.
# @author <fox@nscl.msu.edu>
*/


#ifndef APP_H
#define APP_H
#include <iostream>
#include <map>
#include <vector>
#include <string>



#include "options.h"


class CDataSource;
class CRun;
class CRingItem;

/**
 * @class App
 *    The application class for the scaler sum program.
 */
class App {
public:
    typedef struct _Channel {
        unsigned s_dataSource;
        unsigned s_channel;
        
        int operator>(const struct _Channel& rhs) const;
        int operator<(const struct _Channel& rhs) const;
        int operator==(const struct _Channel& rhs) const;
        int operator!=(const struct _Channel& rhs) const;
        int operator<=(const struct _Channel& rhs) const;
        int operator>=(const struct _Channel& rhs) const;
    } Channel, *pChannel;
    
    typedef struct _ChannelInfo {
        unsigned    s_width;
        std::string s_channelName;
    } ChannelInfo, *pChannelInfo;
    
    typedef enum _States {
        expectingStart, expectingEnd
    } States;
private:
    bool m_omitLabels;
    bool m_flip;
    
    std::vector<std::string>         m_files;
    std::map<Channel, ChannelInfo>   m_channelNames;
    States                           m_state;
    
    std::vector<CRun*>                m_completeRuns;
    CRun*                             m_pCurrentRun;              
public:
    App(struct gengetopt_args_info& args);
    virtual ~App() {}
   
    void operator()();
    void outputResults(std::ostream& out);
    
    // for testing/debugging.
    
    void dumpScalerNames(std::ostream& out);

private:
    void processNameFile(const char* name);
    std::string getScalerName(Channel& ch);
    unsigned    getScalerWidth(Channel& ch);
    
    void processFile(CDataSource& ds);
    std::string makeFileUri(std::string name);

    void begin(CRingItem& item);
    void end();
    void scaler(CRingItem& item);
    
    void outputByRuns(
        std::ostream& out,
        std::map<unsigned, std::map<std::string, uint64_t> >& data
    );
    void outputByScaler(
        std::ostream& out,
        std::map<unsigned, std::map<std::string, uint64_t> >& data
    );
    
    std::string quoteString(std::string s);
    
};

#endif