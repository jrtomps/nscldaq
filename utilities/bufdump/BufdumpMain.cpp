/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <config.h>
#include "BufdumpMain.h"


#include <URL.h>
#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CRingTextItem.h>
#include <CRingScalerItem.h>
#include <CRingBuffer.h>
#include <StringsToIntegers.h>
#include "CDataSource.h"
#include "CRingDataSource.h"
#include "CFileDataSource.h"
#include "CRingPhysicsEventCountItem.h"
#include "dumperargs.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdio.h>


using namespace std;

///////////////////////////////////////////////////////////////////////////////////
//
// Constructor and destructor
//

/*!
   Construct the object.  No real action occurs until the
   operator() is called, as all of the interseting data must be
   determined by parsing the command line arguments.
*/
BufdumpMain::BufdumpMain() :
  m_ringSource(true),
  m_pDataSource(0),
  m_skipCount(0),
  m_itemCount(0)
{
}

/*!
  Destroy the object:
*/
BufdumpMain::~BufdumpMain()
{
  delete m_pDataSource;

}
///////////////////////////////////////////////////////////////////////////////
//
// Public interface:
//

/*!
   Entry point for the dumper.
   - Parse the arguments.
   - Open the data source.
   - Accept items from the ring and dump them to stdout.

   \param argc   Number of command line arguments.
   \param argv   Array of pointers to command line arguments.

   \return int
   \retval EXIT_SUCCESS - Successful execution
   \retval EXIT_FAILURE - Some problem.. or may just exit.
*/
int
BufdumpMain::operator()(int argc, char** argv)
{
  // parse the arguments:

  gengetopt_args_info parse;
  cmdline_parser(argc, argv, &parse);

  // there should be no unnamed arguments. if there are,
  // print the help text and exit:

  if (parse.inputs_num > 0) {
    std::cerr << "Only switches are allowed, not command line parameters\n";
    cmdline_parser_print_help();
    return EXIT_FAILURE;
  }

  // figure out the sample/exclusion vectors:

  vector<uint16_t> sample;
  vector<uint16_t> exclude;
  vector<int>      s;
  if (parse.sample_given) {
    try {
      s = stringListToIntegers(string(parse.sample_arg));
    }
    catch (...) {
      cerr << "Invalid value for --sample, must be a list of item types was: "
	   << string(parse.sample_arg) << endl;
      return EXIT_FAILURE;
    }
    for(int i=0; i < s.size(); i++) {
      sample.push_back(s[i]);
    }
  }
  
  vector<int> e;
  if (parse.exclude_given) {
    try {
      e = stringListToIntegers(string(parse.exclude_arg));
    }
    catch (...) {
      cerr << "Invalid value for --exclude, must be a list of item types was: "
	   << string(parse.sample_arg) << endl;
      return EXIT_FAILURE;
      
    }
    for (int i = 0; i < e.size(); i++) {
      exclude.push_back(e[i]);
    }
  }
    

  // figure out what sort of data source we have:

  string sourceName = defaultSource();
  if (parse.source_given) {
    sourceName = parse.source_arg;
  }

  
  CDataSource* pSource;
  try {
    m_pDataSource = new URL(sourceName);
    if (m_pDataSource->getProto() == string("file")) {
      pSource = new CFileDataSource(*m_pDataSource, exclude);
      m_ringSource = false;
    }
    else if (m_pDataSource->getProto() == string("tcp")) {
      pSource = new CRingDataSource(*m_pDataSource, sample, exclude);
      m_ringSource = true;
    }
    else {
      cerr << "--source url must be either tcp: (ringbuffer) or file: (event file) and was "
	   << m_pDataSource->getProto() << endl;
      return EXIT_FAILURE;
    }
  }
  catch (CException& e) {
    std::cerr << "Failed to open data source " << sourceName << std::endl;
    std::cerr << e.ReasonText() << std::endl;
    throw EXIT_FAILURE;
  }
  catch (std::string msg) {
    std::cerr << "Failed to open data source " << sourceName << std::endl;
    std::cerr << msg << std::endl;
    throw EXIT_FAILURE;
  }
  catch (const char* msg) {
    std::cerr << "Failed to open data source " << sourceName << std::endl;
    std::cerr << msg << std::endl;
    throw EXIT_FAILURE;
  }
  catch (...) {
    std::cerr << "Unanticipated exception attempting to create data source: " << std::endl;
    throw;
  }

  // Can't have sample and file data source:

  if (parse.sample_given && !m_ringSource) {
    cerr << "--source is a file, and --sample is given which is not allowed\n";
    return EXIT_FAILURE;
  }

  // We can now actually get stuff from the ring..but first we need to set the
  // skip and item count:

  if (parse.skip_given) {
    if (parse.skip_arg < 0) {
      cerr << "--skip value must be >= 0 but is "
	   << parse.skip_arg << endl;
      return EXIT_FAILURE;
    }
    m_skipCount = parse.skip_arg;

  }
  if (parse.count_given) {
    if (parse.count_arg < 0) {
      cerr << "--count value must be >= 0 but is "
	   << parse.count_arg << endl;
      return EXIT_FAILURE;
    }

    m_itemCount = parse.count_arg;
  }

  // Skip any items that need to be skipped:

  while (m_skipCount) {
    CRingItem* pSkip = pSource->getItem();
    delete pSkip;
    m_skipCount--;
  }

  size_t numToDo = m_itemCount;
  bool done = false;
  while (!done) {
    CRingItem *pItem = pSource->getItem();
    if (pItem) {
      processItem(*pItem);
      delete pItem;
      
      numToDo--;
      
      if ((m_itemCount != 0) && (numToDo == 0)) done = true;
    } else {
      done = true;
    }
  }

  return EXIT_SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////////
//
//   Utilities.

/*
** Process an item recieved from the ring.
** At this level, we just need to figure out what type of item we have
** cast construct a stand in for it, if necessary, and 
** dispatch to the appropriate dumper member function:
**
** Parameters:
**    item - Reference to the item to dump.
*/
void
BufdumpMain::processItem(const CRingItem& item)
{

  cout << "-----------------------------------------------------------\n";
  switch (item.type()) {
  case BEGIN_RUN:
  case END_RUN:
  case PAUSE_RUN:
  case RESUME_RUN:
    // state change:
    {
      CRingStateChangeItem stateChange(item);
      dumpStateChangeItem(cout, stateChange);
    }
    break;
   
  case PACKET_TYPES:
  case MONITORED_VARIABLES:
    {
      CRingTextItem textItem(item);
      dumpStringListItem(cout, textItem);
    }
    break;

  case INCREMENTAL_SCALERS:
    {
      CRingScalerItem scaler(item);
      dumpScalerItem(cout, scaler);
    }
    break;

  case PHYSICS_EVENT:
    {
      dumpPhysicsItem(cout, item);
      break;
    }
  case PHYSICS_EVENT_COUNT:
    {
      CRingPhysicsEventCountItem eventCount(item);
      dumpEventCountItem(cout, eventCount);
    }
    break;
  default:
    {
      dumpUnknownItem(cout, item);
    }
  }
}
/*
** Dump a state change item.
** 
** Paramters:
**   out      - stream to which the dump should go.
**   item     - Ring state change item to dump.
*/
void
BufdumpMain::dumpStateChangeItem(ostream& out, const CRingStateChangeItem& item)
{
  uint32_t run       = item.getRunNumber();
  uint32_t elapsed   = item.getElapsedTime();
  string   title     = item.getTitle();
  string   timestamp = timeString(item.getTimestamp());



  out <<  timestamp << " : Run State change : ";
  switch (item.type()) {
  case BEGIN_RUN:
    out << " Begin Run ";
    break;
  case END_RUN:
    out << "End Run ";
    break;
  case PAUSE_RUN:
    out << "Pause Run ";
    break;
  case RESUME_RUN:
    out << "Resume Run ";
    break;
  }
  out << "  at " << elapsed << " seconds into the run \n";
  out << "Title    : " << title << endl;
  out << "RunNumber: " << run   << endl;
}

/*
** Dump a string list item.
** Parmeters:
**   out    - Output stream to which the item will be dumped.
**   item   - Reference to the item to dump.
*/
void
BufdumpMain::dumpStringListItem(ostream& out, const CRingTextItem& item)
{
  uint32_t elapsed  = item.getTimeOffset();
  string   time     = timeString(item.getTimestamp());
  vector<string> strings = item.getStrings();

  out << time << " : Documentation item ";
  switch (item.type()) {
   
  case PACKET_TYPES:
    out << " Packet types: ";
    break;
  case MONITORED_VARIABLES:
    out << " Monitored Variables: ";
    break;
    
  }
  out << elapsed << " seconds in to the run\n";
  for (int i = 0; i < strings.size(); i++) {
    out << strings[i] << endl;
  }

}
/*
** Dump a scaler item.
**
** Parameters:
**   out   - the file to which to dump.
**   item  - reference to the item to dump.
*/
void
BufdumpMain::dumpScalerItem(ostream& out, const CRingScalerItem& item)
{
  uint32_t end   = item.getEndTime();
  uint32_t start = item.getStartTime();
  string   time  = timeString(item.getTimestamp());
  vector<uint32_t> scalers = item.getScalers();

  float   duration = static_cast<float>(end - start);

  out << time << " : Incremental scalers:\n";
  out << "Interval start time: " << start << " end: " << end << " seconds in to the run\n\n";
  

  out << "Index         Counts                 Rate\n";
  for (int i=0; i < scalers.size(); i++) {
    char line[128];
    double rate = (static_cast<double>(scalers[i])/duration);

    sprintf(line, "%5d      %9d                 %.2f\n",
	    i, scalers[i], rate);
    out << line;
  }

}

/*
** Dump a physics item.  
**
** Parameters:
**   out  - The output file on which to dump the item.
**   item - The item to dump.
**
*/
void
BufdumpMain::dumpPhysicsItem(ostream& out, const CRingItem& item)
{
  uint32_t  bytes = item.getBodySize();
  uint32_t  words = bytes/sizeof(uint16_t);
  const uint16_t* body  = reinterpret_cast<const uint16_t*>(const_cast<CRingItem&>(item).getBodyPointer());

  out << "Event " << bytes << " bytes long\n";

  int  w = out.width();
  char f = out.fill();

  
  for (int i =1; i <= words; i++) {
    char number[32];
    sprintf(number, "%04x ", *body++);
    out << number;
    if ( (i%8) == 0) {
      out << endl;
    }
  }
  out << endl;
  
  
}
/*
** Dumps an item that describes the number of accepted triggers.
**
** Parameters:
**   out  - Where to dump the item.
**   item - The item itself.
**
*/
void
BufdumpMain::dumpEventCountItem(ostream& out, const CRingPhysicsEventCountItem& item)
{
  string   time   = timeString(item.getTimestamp());
  uint32_t offset = item.getTimeOffset();
  uint64_t events = item.getEventCount();


  out << time << " : " << events << " Triggers accepted as of " 
      << offset << " seconds into the run\n";
  out << " Average accepted trigger rate: " 
      <<  (static_cast<double>(events)/static_cast<double>(offset))
      << " events/second \n";
}
/*
**  Dump an item of some unknown type.  Just a byte-wise binary dump.
**
** Parameter:
**   out   - stream to which to dump the item.
**   item  - Item to dump.
*/
void
BufdumpMain::dumpUnknownItem(ostream& out, const CRingItem& item)
{
  uint16_t type  = item.type();
  uint32_t bytes = item.getBodySize();
  uint8_t* p     = reinterpret_cast<uint8_t*>(const_cast<CRingItem&>(item).getBodyPointer());

  out << "Unknown item type: " << type << endl;
  out << "Body size        : " << bytes << endl;
  out << "Body:\n";


  for (int i =1; i <= bytes; i++) {
    char item[16];
    sprintf(item, "%02x ", *p++);
    out << item;
    if ((i%16) == 0) {
      out << endl;
    }
  }
  out << endl;


}

/*
** Return the default source URL which is 
** tcp://localhost/username
**
*/
string
BufdumpMain::defaultSource() const
{
  return CRingBuffer::defaultRingUrl();
			
}

/*
** Returns the time string associated with some time_t
*/
string
BufdumpMain::timeString(time_t theTime) const
{

  string result(ctime(&theTime));
  
  // For whatever reason, ctime appends a '\n' on the end.
  // We need to remove that.

  result.erase(result.size()-1);

  return result;
}
