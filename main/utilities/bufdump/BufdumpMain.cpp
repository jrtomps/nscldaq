/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <config.h>
#include "BufdumpMain.h"


#include <URL.h>
#include <CRingBuffer.h>
#include <CDataSource.h>
#include <CDataSourceFactory.h>
#include <V12/CRingItemFactory.h>
#include <V12/CRawRingItem.h>
#include <V12/CRingItem.h>
#include <V12/StringsToIntegers.h>
#include <V12/CRingScalerItem.h>
#include <RingIOV12.h>

#include <CAllButPredicate.h>
#include <CRingSelectPredWrapper.h>

#include <Exception.h>

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

using namespace DAQ;
using namespace DAQ::V12;
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
  m_skipCount(0),
  m_itemCount(0)
{
}

/*!
  Destroy the object:
*/
BufdumpMain::~BufdumpMain()
{
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

  // Set the scaler format mask... note 32 is a bit pathalogical in a 
  // for uint32_t values...note also the max is 32 bits:

  if ((parse.scaler_width_arg > 32) ||  (parse.scaler_width_arg <= 0)) {
    std::cerr << "The --scaler-width flag value must be greater than zero and 32 or less, was: " 
	      << parse.scaler_width_arg << std::endl;
    return EXIT_FAILURE;
  } else if (parse.scaler_width_arg == 32) {
    CRingScalerItem::m_scalerFormatMask = 0xffffffff;
  } else {
    CRingScalerItem::m_scalerFormatMask = (1 << parse.scaler_width_arg) - 1;
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

  
  CDataSourceUPtr pSource;
  try {
    pSource = CDataSourceFactory::makeSource(sourceName);

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

  auto pPredicate = std::shared_ptr<CAllButPredicate>(new CAllButPredicate);

  for (int i=0; i < exclude.size(); i++) {
    pPredicate->addExceptionType(exclude[i]);
  }
  for (int i=0; i < sample.size(); i++) {
    pPredicate->addExceptionType(sample[i], true);
  }

  CRingSelectPredWrapper predicate(pPredicate);
  CRawRingItem item;
  while (m_skipCount && !pSource->eof()) {
      readItemIf(*pSource, item, predicate);
      m_skipCount--;
  }
  size_t numToDo = m_itemCount;
  bool done = false;

  while (!done && !pSource->eof()) {
      readItemIf(*pSource, item, predicate);
      if (!pSource->eof()) {
        processItem(item);

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
BufdumpMain::processItem(const CRawRingItem& item)
{

  cout << "-----------------------------------------------------------\n";
  auto pActualItem = CRingItemFactory::createRingItem(item);
  cout << pActualItem->toString();

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
