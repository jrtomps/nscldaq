// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include "CEndRunInfoFactory.h"
#include <io.h>
#include <DataFormat.h>
#include <DataFormat10.h>
#include "CEndRunInfo10.h"
#include "CEndRunInfo11.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <exception>
#include <stdexcept>
#include <CDataFormatItem.h>
#include <time.h>
#include <string.h>


class ErInfoFactoryTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ErInfoFactoryTests);
  CPPUNIT_TEST(explicit10);
  CPPUNIT_TEST(explicit11);
  CPPUNIT_TEST(explicitbad);
  CPPUNIT_TEST(fromfile11);
  CPPUNIT_TEST(fromfile10);
  CPPUNIT_TEST(fromfileunrecog);
  CPPUNIT_TEST(fromfileempty);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void explicit10();
  void explicit11();
  void explicitbad();
  
  void fromfile11();
  void fromfile10();
  void fromfileunrecog();
  void fromfileempty();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ErInfoFactoryTests);

// Explicitly create a version 10 endinfo:

void ErInfoFactoryTests::explicit10() {
  //  We'll use the /dev/null file:
  
  int fd = open("/dev/null", O_RDONLY);
  
  CEndRunInfo* pEr;
  CPPUNIT_ASSERT_NO_THROW(
    pEr = CEndRunInfoFactory::create(CEndRunInfoFactory::nscldaq10, fd)
  );
  
  ASSERT(pEr);
  CEndRunInfo10* pE10 = dynamic_cast<CEndRunInfo10*>(pEr);
  ASSERT(pE10);                      // null if not castable.
  
  close(fd);
  delete pEr;
  
                
}
// Explicitly create a version 11 endinfo

void ErInfoFactoryTests::explicit11()
{
  int fd = open("/dev/null", O_RDONLY);
  
  CEndRunInfo* pEr;
  CPPUNIT_ASSERT_NO_THROW(
    pEr = CEndRunInfoFactory::create(CEndRunInfoFactory::nscldaq11, fd)
  );
  
  ASSERT(pEr);
  CEndRunInfo11* pE11 = dynamic_cast<CEndRunInfo11*>(pEr);
  ASSERT(pE11);
  
  close(fd);
  delete pE11;
}

// bad nscldaq on expliciti create:

void ErInfoFactoryTests::explicitbad()
{
  CPPUNIT_ASSERT_THROW(
    CEndRunInfoFactory::create(
      static_cast<CEndRunInfoFactory::DAQVersion>(234), 0),
    std::domain_error
  );
}
// given a  file that leads with an 11.x format create and  11.0
// endrun info object.

void ErInfoFactoryTests::fromfile11()
{
    // Create the tmp file.
  
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  
  // Write an 11.x ring format item:
  
  CDataFormatItem item;
  io::writeData(fd, item.getItemPointer(), item.getItemPointer()->s_header.s_size);
  
  // Should not need to rewind.
  
  CEndRunInfo* pObj = CEndRunInfoFactory::create(fd);
  close(fd);
  
  ASSERT(pObj);
  CEndRunInfo11* p11 = dynamic_cast<CEndRunInfo11*>(pObj);

  ASSERT(p11);
  
  delete pObj;

}
// Given a file without a format it's assumed to be a 10.

void ErInfoFactoryTests::fromfile10()
{
  // create a temp file:


  
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  
  //  Write an nscldaq10 begin run item:
  
  NSCLDAQ10::StateChangeItem item;
  item.s_header.s_size = sizeof(item);
  item.s_header.s_type = NSCLDAQ10::BEGIN_RUN;
  item.s_runNumber     = 10;
  item.s_timeOffset    = 0;
  item.s_Timestamp     = time(NULL);
  strcpy(item.s_title, "This is a run");
  
  io::writeData(fd, &item, sizeof(item));
  
  CEndRunInfo* pObj = CEndRunInfoFactory::create(fd);
  ASSERT(pObj);
  CEndRunInfo10* p10 = dynamic_cast<CEndRunInfo10*>(pObj);
  ASSERT(p10);
  
  close(fd);
  delete pObj;
}
// file with a 12.0 format record

void ErInfoFactoryTests::fromfileunrecog()
{
 // create a temp file:
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  
  // Make a format item for 12.0
  
  DataFormat item;
  item.s_header.s_size = sizeof(item);
  item.s_header.s_type = RING_FORMAT;
  item.s_mbz           = 0;
  item.s_majorVersion  = 12;
  item.s_minorVersion  = 0;
  
  io::writeData(fd, &item, sizeof(item));
  
  //  now the factory should throw a domain error:
  
  CPPUNIT_ASSERT_THROW(
    CEndRunInfoFactory::create(fd),
    std::domain_error
  );
}
// Empty file looks like 10.2 with no end run records.

void ErInfoFactoryTests::fromfileempty()
{
  int fd = open("/dev/null", O_RDONLY);
  CEndRunInfo* er = CEndRunInfoFactory::create(fd);
  
  EQ(unsigned(0), er->numEnds());
  
  ASSERT(dynamic_cast<CEndRunInfo10*>(er));
}