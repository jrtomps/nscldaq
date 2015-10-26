// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>


#include "CEndRunInfo11.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <CRingStateChangeItem.h>
#include <CRingItem.h>
#include <DataFormat.h>
#include <io.h>
#include <time.h>
#include <stdexcept>

class TestEndInfo11 : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestEndInfo11);
  CPPUNIT_TEST(emptyFile);
  CPPUNIT_TEST(oneWithBh);
  CPPUNIT_TEST(oneWoBH);
  CPPUNIT_TEST(noSuchEr1);
  CPPUNIT_TEST(nobodyHeaderThrows);

  CPPUNIT_TEST(twoWithBh);
  // CPPUNIT_TEST(twoWoBh);
  // CPPUNIT_TEST(twoWithMixed);

  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void emptyFile();
  void oneWithBh();
  void oneWoBH();
  void noSuchEr1();
  void nobodyHeaderThrows();
  
  void twoWithBh();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestEndInfo11);

void TestEndInfo11::emptyFile() {
  int fd = open("/dev/null",   O_RDONLY);
  CEndRunInfo11 er(fd);
  close(fd);                                        // By now we're done.
  
  EQ(0U, er.numEnds());                  // /dev/null has no end records.
}

// File with a single end run record.

void TestEndInfo11::oneWithBh()
{
  // Create the tmp file and write a single End Run with a body header.
  
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  time_t now = time(NULL);
  CRingStateChangeItem end(
    666, 1, 2,
    END_RUN, 1234, 456, now, "This is a title"
  );
  RingItem* pItem = end.getItemPointer();
  uint32_t itemSize = pItem->s_header.s_size;
  io::writeData(fd, pItem, itemSize);
  
  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  EQ(1U, er.numEnds());
  
  // See if the contents are right:
  
  ASSERT(er.hasBodyHeader());
  EQ(uint64_t(666), er.getEventTimestamp());
  EQ(uint32_t(1), er.getSourceId());
  EQ(uint32_t(2),  er.getBarrierType());
  
  EQ(uint32_t(1234), er.getRunNumber());
  EQ(float(456.0), er.getElapsedTime());
  EQ(std::string("This is a title"), er.getTitle());
  EQ(now, er.getTod());
}

void TestEndInfo11::oneWoBH()
{
  // Create the tmp file and write a single End Run with a body header.
  
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  time_t now = time(NULL);
  CRingStateChangeItem end(
    END_RUN, 1234, 456, now, "This is a title"
  );
  RingItem* pItem = end.getItemPointer();
  uint32_t itemSize = pItem->s_header.s_size;
  io::writeData(fd, pItem, itemSize);
  
  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  EQ(1U, er.numEnds());
  
  // See if the contents are right:
  
  ASSERT(!er.hasBodyHeader());
  
  EQ(uint32_t(1234), er.getRunNumber());
  EQ(float(456.0), er.getElapsedTime());
  EQ(std::string("This is a title"), er.getTitle());
  EQ(now, er.getTod());  
}

void TestEndInfo11::noSuchEr1()
{
  int fd = open("/dev/null",   O_RDONLY);
  CEndRunInfo11 er(fd);
  close(fd);                                        // By now we're done.
  
  // Asking for any info from any er should throw:
  
  
  CPPUNIT_ASSERT_THROW(
    er.hasBodyHeader(0),
    std::range_error
  );
}
void TestEndInfo11::nobodyHeaderThrows()
{
  // Create the tmp file and write a single End Run with a body header.
  
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  time_t now = time(NULL);
  CRingStateChangeItem end(
    END_RUN, 1234, 456, now, "This is a title"
  );
  RingItem* pItem = end.getItemPointer();
  uint32_t itemSize = pItem->s_header.s_size;
  io::writeData(fd, pItem, itemSize);
  
  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  // Getting any body header info throws strings:
  
  CPPUNIT_ASSERT_THROW(
    er.getEventTimestamp(),
    std::string
  );
  CPPUNIT_ASSERT_THROW(
    er.getSourceId(),
    std::string
  );
  CPPUNIT_ASSERT_THROW(
    er.getBarrierType(),
    std::string
  );
}

// This run has two end run records with body headers.
//
void TestEndInfo11::twoWithBh()
{
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  time_t now = time(NULL);
  
  // First one:
  
  CRingStateChangeItem end(
    666, 1, 2,
    END_RUN, 1234, 456, now, "This is a title"
  );
  RingItem* pItem = end.getItemPointer();
  uint32_t itemSize = pItem->s_header.s_size;
  io::writeData(fd, pItem, itemSize);
    
  // Second one:
  
  CRingStateChangeItem end2(
    676, 2, 2,
    END_RUN, 1234, 456, now, "This is a title"
  );
  pItem    = end2.getItemPointer();
  itemSize = pItem->s_header.s_size;
  io::writeData(fd, pItem, itemSize);
  
  // Rewind the fd and create the end run info around it.
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo11 er(fd);
  close(fd);                          // should be done now.
  
  // Now check what we found:
  
  EQ(2U, er.numEnds());
  
  ASSERT(er.hasBodyHeader(0));
  ASSERT(er.hasBodyHeader(1));
  
  // These should be enough to tell us that we are getting the
  // right items.
  
  EQ(uint64_t(666), er.getEventTimestamp(0));
  EQ(uint64_t(676), er.getEventTimestamp(1));
  
  EQ(uint32_t(1), er.getSourceId());
  EQ(uint32_t(2), er.getSourceId(1));
  
  
}
