
#ifndef CDATASINKFACTORY_H
#define CDATASINKFACTORY_H

#include <string>

class CDataSink;


/**! Factory class for constructing CDataSink objects
*
* When provided a universal resource identifier (URI), this 
* will return the appropriate type of data sink.
*
* Supported sinks at the present are:
*   CFileDataSink   - specified by the file:// protocol
*                     (stdout can be specified as file:///stdout or - )
*
* To be supported in the future:
*   CRingDataSink   - TODO...
*
*/
class CDataSinkFactory
{
  public:
    /**! 

      Parse the argument and return the proper type of sink
    */
    CDataSink* makeSink(std::string uri);

  private:
    /**!
       Create a file data sink for the specified file    
    */
    CDataSink* makeFileSink(std::string fname); 

    /**!
       Create a ring data sink with the specified name
    */
    CDataSink* makeRingSink(std::string ringname); 
};

#endif
