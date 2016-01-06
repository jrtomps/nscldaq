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
# @file   App.cpp
# @brief  Application driver class for offline scaler sum program.
# @author <fox@nscl.msu.edu>
*/

#include "App.h"
#include "CRun.h"

#include <CDataSource.h>
#include <CDataSourceFactory.h>
#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingItemFactory.h>
#include <CDataFormatItem.h>
#include <DataFormat.h>

#include <ErrnoException.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>



/**
 * constructor
 *
 * @param args - Command line arguments processed gengetopt.
 */
App::App(struct gengetopt_args_info& args) :
    m_omitLabels(false),
    m_flip(false),
    m_state(App::expectingStart),
    m_pCurrentRun(0)
{
    // Process the cooked parameters:
    
    if (args.omit_labels_given) m_omitLabels = true;
    if (args.flip_given)        m_flip       = false;
    if (args.name_file_given) {
        processNameFile(args.name_file_arg);
    }
    
    for (int i = 0; i < args.inputs_num; i++) {
       m_files.push_back(args.inputs[i]);
    }
}

// TODO:   Implement destructor to kill off m_completeRuns.

/**
 * operator()
 *     Runs the analysis. Either process all the lines or 

/*---------------------------------------------------------------------------
 * public methods
 */

/**
 * operator()
 *     Process all of the input:
 *     -   If there are no input files, a data source for stdin is created
 *         and processed.
 *     -   If there are input files those are processed.
 *  @note living above all of this is a simple state machine with the state:
 *        -  expectingStart - Looking for a begin run.
 *        -  expectingEnd   - Processing scalers until an end run.
 */
void
App::operator()()
{
    std::vector<uint16_t> dummy;
    if (m_files.size() == 0) {
        std::unique_ptr<CDataSource>
            pDs(CDataSourceFactory::makeSource("-", dummy, dummy));
        processFile(*pDs);
    } else {
        for(auto p = m_files.begin(); p != m_files.end(); p++) {
            try {
                std::string uri = makeFileUri(*p);
                std::unique_ptr<CDataSource> 
                    pDs(CDataSourceFactory::makeSource(uri, dummy, dummy));
                processFile(*pDs);
            }
            catch (CErrnoException& e) {
                std::string msg = "Unable to process file : ";
                msg += *p;
                msg += " : ";
                msg += e.ReasonText();
                throw std::runtime_error(msg);
            }
        }
    }
    // If the state is not expectingStart  we've got a partial run to warn
    // about
    
    if ((m_state != expectingStart) && m_pCurrentRun) {
        std::cerr << "Warning - end of all input in the middle of a run.\n";
        std::cerr << "Saving the sums for this partial run: " << m_pCurrentRun->getRun();
        std::cerr << std::endl;
        m_completeRuns.push_back(m_pCurrentRun);
        m_pCurrentRun = 0;
    }
}

/**
 *  processFile
 *     Process the data on an input source. Note that a input source could be
 *     part of a run, all of a run or (e.g. in the case of stdin) several
 *     runs.  Here's a rough go of the action.
 *
 *     I'd like to require a data source begin with a data format item however
 *     we can't since only the first segment of each multi file run has them
 *     at present.
 *
 *     The system starts out expecting the beginning of a run.
 *     Items are skipped until a begin run is found.  At that point in time
 *     a new CRun object (m_pCurrentRun) is created and the state is set to
 *     expectingEnd
 *
 *     In expectingEnd we process scaler items summing them in or setting them
 *     (if the scalers are not incremental).  This continues until an end
 *     run item is seen at which time the current run object is appended to
 *     the completed runs vector and we transition back into expectingStart.
 *
 *     There are a few exceptional cases:
 *     -  scalers seen in expectingStart are ignored.
 *     -  Begin items seen in expectingEnd give a warning but act like an
 *        end run followed by the start we got.
 *     -  Data source may hit the end of data in either state (think about
 *        multi file segmented runs here).
 *
 * @param ds - data source that provides us with ring items.
 * @note - Online data can be collected by processin data from stdin where
 *         stdin is piped from ringtostdout.  If you do this, since the program
 *         outputs data only once the data sources are exhausted, you must
 *         get output by killing the data source so that we see the EOF.
 */
void
App::processFile(CDataSource& ds) {
    try {
        CRingItem* pRawItem;
        while (pRawItem = ds.getItem()) {
            // What we do depends on type and state:
            
            switch (pRawItem->type()) {
            case BEGIN_RUN:
                if(m_state == expectingEnd) {
                    std::cerr << "Warning, got a begin run in the middle of processing run ";
                    std::cerr << m_pCurrentRun->getRun() << std::endl;
                    std::cerr << "Saving partial run  sums and continuing.";
                    end();
                }
                begin(*pRawItem);
                m_state = expectingEnd;
                break;
            case END_RUN:
                if (m_state == expectingStart) {
                    std::cerr << "Warning - got an end run while expecting a begin\n";
                    std::cerr << "Continuing processing\n";
                    
                    // probably don't have one but in case we do:
                    
                    delete m_pCurrentRun;
                    m_pCurrentRun = 0;
                } else {
                    end();
                    m_state = expectingStart;
                }
                break;
            case PERIODIC_SCALERS:
                if (m_state == expectingEnd) {
                    scaler(*pRawItem);
                }
                break;
            defaut:
                break;
            }
            
            delete pRawItem;       // - it was dynamic.
        }
        // Null item be sure this is an EOF else throw the errno.
        
        if (errno != 0) {
            throw int(errno);
        }
    }
    catch (int i) {                      // getItem can throw ints.
        std::string msg = "Error processing a data source: ";
        msg += strerror(i);
        throw std::runtime_error(msg);
    }
}

/**
 * dumpScalerNames
 *    For debugging purposes, dumps the scaler name map to the
 *    specified stream.
 *
 *    @param f  - output stream to which the data will be dumped:
 */
void
App::dumpScalerNames(std::ostream& f)
{
    for(auto p = m_channelNames.begin(); p != m_channelNames.end(); p++) {
        Channel        c = p->first;
        std::string name = (p->second).s_channelName;
        
        f << "Channel: " << c.s_dataSource << '.' << c.s_channel << " - " <<
            name << " " << (p->second).s_width << " bits\n";
    }
    
}

/*--------------------------------------------------------------------------
 * private methods
 */

/**
 * processNameFile
 *    Given a file of scaler names, fill in the scaler channel -> name map.
 *
 *  @param name - name of input file.
 *  @throws     - std::invalid_argument - if the name file does not exist.
 *  @note - May produce other deal breaker errors as (eg. std::runtime_error)
 *  @note - May produce warnings on std::cerr (e.g. for redefining a scaler).
 */

void
App::processNameFile(const char* name)
{
    std::ifstream nameFile(name);
    if (nameFile.fail()) {
        std::string msg = "Could not open scaler name file: ";
        msg += name;
        throw std::invalid_argument(msg);
    }
    
    Channel chanSpec;
    ChannelInfo info;
    
    
    // TODO: parse lines via intermediate string so we can give better
    //       diagnostics.
    
    while(! nameFile.eof()) {
        std::string line;
        std::getline(nameFile, line, '\n');
        if(line.size() == 0) return;
        std::stringstream sline(line);
        
        
        sline >> chanSpec.s_dataSource >> chanSpec.s_channel >> info.s_width;
        if(sline.fail()) {
            throw std::runtime_error("Invalid line in scaler name file");
        }
        std::getline(sline, info.s_channelName, '\n');
        if (m_channelNames.find(chanSpec) != m_channelNames.end()) {
            std::cerr << "Warning redefining channel " << chanSpec.s_dataSource
                << '.' << chanSpec.s_channel << " name to be: " << info.s_channelName << std::endl;
        }
        m_channelNames[chanSpec] = info;
    }
    
    
}
/**
 * getScalerName
 *    Returns the name of a channel.  The name is looked up in the m_channelNames
 *    map.  If not found an default name is generated and returned (the resulting)
 *    scaler is set to the 32 bit width default.
 *
 *  @param ch  - Channel specification (source and channel number).
 *  @return std::string - the channel label.
 *  @note   m_channelNames may be modified.
 */
std::string
App::getScalerName(App::Channel& ch)
{
    // If necessary create/insert a new name:
    
    if (m_channelNames.find(ch) == m_channelNames.end()) {
        ChannelInfo info;
        std::stringstream name;
        name << "Scaler-" << ch.s_dataSource << '.' << ch.s_channel;
        info.s_channelName = name.str();
        info.s_width = 32;
        m_channelNames[ch] = info;
    }
    
    return m_channelNames[ch].s_channelName;
}
/**
 * getScalerWidth
 *    Returns the width of a channel.  This is not done that efficiently
 *    - first the name is gotten to ensure the existence in the map
 *    - then the width is retrned
 * @return unsigned
 */
unsigned
App::getScalerWidth(Channel& ch)
{
    getScalerName(ch);               // Now we know it's in the map.
    return m_channelNames[ch].s_width;
}
/**
 * makeFileUri
 *    Turn a filename into a URI
 *
 *  @param name - name of the file.
 *  @return std::string - URI pointing at the file.
 */
std::string
App::makeFileUri(std::string name)
{
      char*  fullPath = realpath(name.c_str(), NULL);
      if (fullPath) {
        std::string uri = "file://";
        uri += fullPath;
        free(fullPath);
        return uri;
      } else {
        // Error creating the path:
        
        std::string errnomsg = strerror(errno);
        std::string msg = "Unable to create a URI for ";
        msg += name;
        msg += " : ";
        msg += errnomsg;
        throw std::runtime_error(msg);
        
      }
}

/**
 * begin
 *    Begin run processing
 *    - Get the run number from the item.
 *    - Create a new CRun object at m_pCurrent Run.
 *    - Redundant but set the state to expectingEnd.
 * @param item - the undifferentiated item.
 */
void
App::begin(CRingItem& item)
{
    CRingStateChangeItem* pBegin =
        dynamic_cast<CRingStateChangeItem*>(CRingItemFactory::createRingItem(item));
    m_pCurrentRun = new CRun(pBegin->getRunNumber());
    
    delete pBegin;
    m_state = expectingEnd;
}
/**
 * end
 *    End of a run.  The current run object is saved in the list of
 *    completely processed runs, its pointer member is zeroed and
 *    the state is set to expectingStart:
 */
void
App::end()
{
    m_completeRuns.push_back(m_pCurrentRun);
    m_pCurrentRun = 0;
    m_state = expectingStart;
}

/**
 * scaler
 *    Process a scaler item
 *    - Convert the item to a scaler item.
 *    - Pull out the incremental flag and the data source
 *    - Pass the increments to the run one by one...getting the scaler width
 *      as we do.
 * @param item - references the ring item that is being processed.
 */
void
App::scaler(CRingItem& item)
{
    CRingScalerItem* pScaler =
        reinterpret_cast<CRingScalerItem*>(CRingItemFactory::createRingItem(item));
        
    // Source id is 0 if there's no body header:
    
    unsigned srcId;
    if (pScaler->hasBodyHeader()) {
        srcId = pScaler->getSourceId();
    } else {
        srcId = 0;
    }
    // Figure out if the scalers are incremental or not and get the counters.
    
    bool incremental = pScaler->isIncremental();
    std::vector<uint32_t> scalers = pScaler->getScalers();
    
    // Make a Channel struct and use it to iterate over the scalers in the
    // vector:
    
    Channel ch = {srcId, 0}; 
    for (ch.s_channel = 0; ch.s_channel < scalers.size(); ch.s_channel++) {
        unsigned width = getScalerWidth(ch);
        m_pCurrentRun->update(
            ch.s_dataSource, ch.s_channel, scalers[ch.s_channel],
            incremental, width
        );
    }
}

/*-------------------------------------------------------------------------
 * In order to do a map whose keys are Channel structs wwe need to impose
 * a collation ordering.   We do that by first ordering by data source
 * and then by channel within the data source
 *   Here are the ordering functions std::map requires:
 */

// Strict ordering:

int
App::Channel::operator>(const App::Channel& rhs) const {
    if (s_dataSource > rhs.s_dataSource) return 1;
    if (s_dataSource == rhs.s_dataSource) {
        return s_channel > rhs.s_channel;
    }
    return 0;
}

int
App::Channel::operator<(const App::Channel& rhs) const {
    if (s_dataSource < rhs.s_dataSource) return 1;
    if (s_dataSource == rhs.s_dataSource) {
        return s_channel < rhs.s_channel;
    }
    return 0;
}

// Equality/inequality:

int
App::Channel::operator==(const App::Channel& rhs) const {
    return ((s_dataSource == rhs.s_dataSource) && (s_channel == rhs.s_channel));
}
int
App::Channel::operator!=(const App::Channel& rhs) const {
    return !(*this == rhs);
}

// Partial in equalities:

int
App::Channel::operator<=(const App::Channel& rhs) const {
    return !(*this > rhs);
}
int
App::Channel::operator>=(const App::Channel& rhs) const {
    return !(*this < rhs);
}


