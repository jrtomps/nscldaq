

#ifndef CFILTERMAIN_H
#define CFILTERMAIN_H

#include <CMediator.h>
#include "filterargs.h"
#include <vector> 
#include <stdint.h>
#include <CFatalException.h>

class CDataSource;
class CFilter;
class CDataSink;


class CFilterMain
{
  
  private:
    CMediator m_mediator; //!< The mediator
    struct gengetopt_args_info m_argsInfo; //!< The parsed options

  public:
    /**! Constructor
      Constructs a mediator object with a CCaesarFilter
      as the default filter. 
    
      \param argc the number of command line args
      \param argv the command line args

      \throw can throw a CFatalException

     */
    CFilterMain(int argc, char** argv);

    /**! Destructor
      This does absolutely nothing.
    */
    ~CFilterMain() throw();

    /**! Main loop 
      Executes the CMediator::mainLoop() 
    */
    void operator()();

    /**! Append a filter to the mediator's ccomposite filter
      Note that because the filter argument will be used solely 
      to call a clone, it is very important that any derived class
      will provide its own clone method.
  
      \param filter a template of the filter to register
    */
    void registerFilter(CFilter* filter);

  private:
    // Private utility functions 
    CDataSource* constructDataSource();
    CDataSink* constructDataSink();

    std::vector<uint16_t> constructExcludesList();
    std::vector<uint16_t> constructSampleList();
    
};

#endif
