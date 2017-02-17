#ifndef BUFDUMPMAIN_H
#define BUFDUMPMAIN_H

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

#include <vector>
#include <string>
#include <ostream>
#include <cstdint>


// Forward definitiosn:

class URL;

namespace DAQ {
namespace V12 {
class CRawRingItem;
}
}

/*!
  This class is the actual dumper program.  It defines a function object type
  that can be created and invoked from main() to do the job of dumping
  items from some item source on stdout.

  See dumperargs.ggo for information about the switches etc. recognized by this
  program/object.

*/
class BufdumpMain
{
  // private data:
  
private:
  size_t                m_skipCount;   // Number  of items to skip before dumping.
  size_t                m_itemCount;   // Number of items to dump before exiting (0 means infinite).
  std::vector<uint16_t> m_sampleTypes; // Items that should be sampled only (ring buffers).
  std::vector<uint16_t> m_excludeTypes; // Items that should not be dumped at all.
  int                   m_scalerWidth;

  // Canonicals... no need for copy construction etc.
  //
public:
  BufdumpMain();
  virtual ~BufdumpMain();
protected:
  BufdumpMain(const BufdumpMain& rhs);
  BufdumpMain& operator=(const BufdumpMain& rhs);
  int operator==(const BufdumpMain& rhs) const;
  int operator!=(const BufdumpMain& rhs) const;
public:

  // Entry point:

  int operator()(int argc, char** argv);

  //Utilities:

private:
  void processItem(const DAQ::V12::CRawRingItem& item);

  std::string defaultSource() const;   
};

#endif
