// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include <string>
#include <io.h>
#include "CEndRunInfo10.h"
#include <DataFormat10.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdexcept>

class EndInfo10Test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EndInfo10Test);
  CPPUNIT_TEST(emptyFile);
  CPPUNIT_TEST(oneEnd);
  CPPUNIT_TEST(twoEnds);
  CPPUNIT_TEST(bodyHeaderThrows);
  CPPUNIT_TEST(rangeThrows);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void emptyFile();
  void oneEnd();
  void twoEnds();
  void bodyHeaderThrows();
  void rangeThrows();
};

CPPUNIT_TEST_SUITE_REGISTRATION(EndInfo10Test);

void EndInfo10Test::emptyFile() {
  // Open /dev/null and use that as the file:
  
  int fd = open("/dev/null", O_RDONLY);
  CEndRunInfo10 e(fd);
  close(fd);
  
  EQ(unsigned(0), e.numEnds());
}

void EndInfo10Test::oneEnd()
{
  // Open a tempfile and write a single end item.

  
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  
  time_t now = time(NULL);
  NSCLDAQ10::StateChangeItem item;
  item.s_header.s_size = sizeof(item);
  item.s_header.s_type = NSCLDAQ10::END_RUN;
  item.s_runNumber = 1234;
  item.s_timeOffset = 456;
  item.s_Timestamp = now;
  strcpy(item.s_title, "This is a title");
  
  io::writeData(fd, &item, sizeof(item));
  
  // Rewind the file and create an end run info object on it:
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo10 er(fd);
  close(fd);                           // By now we're done
  
  //  Test that we have the right stuff in the er object.
  
  EQ(unsigned(1), er.numEnds());
  ASSERT(!er.hasBodyHeader());     // 10.x never does.
  
  EQ(uint32_t(1234), er.getRunNumber());
  EQ(float(456), er.getElapsedTime());
  EQ(now, er.getTod());
  EQ(std::string("This is a title"), er.getTitle());
}

void EndInfo10Test::twoEnds()
{
  // Open the temp file:
  
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  
  // Create and write the first end run:
  
  time_t now = time(NULL);
  NSCLDAQ10::StateChangeItem item;
  item.s_header.s_size = sizeof(item);
  item.s_header.s_type = NSCLDAQ10::END_RUN;
  item.s_runNumber = 1234;
  item.s_timeOffset = 456;
  item.s_Timestamp = now;
  strcpy(item.s_title, "This is a title");
  
  io::writeData(fd, &item, sizeof(item));
  
  // Slightly modify the item and write it a second time:
  
  item.s_Timestamp = now+10;
  item.s_timeOffset= 466;
  
  io::writeData(fd, &item, sizeof(item));
  
  // Create the ER info item and close the file:
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo10 er(fd);
  close(fd);                           // By now we're done
  
  // Fish info out of the er info object:
  
  EQ(unsigned(2), er.numEnds());
  
  // Distinguish between the two items:
  
  EQ(float(456), er.getElapsedTime());
  EQ(float(466), er.getElapsedTime(1));
  
  EQ(now, er.getTod());
  EQ(now+10, er.getTod(1));
}

void EndInfo10Test::bodyHeaderThrows()
{
  // Open the temp file:
  
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  
  // Create and write the first end run:
  
  time_t now = time(NULL);
  NSCLDAQ10::StateChangeItem item;
  item.s_header.s_size = sizeof(item);
  item.s_header.s_type = NSCLDAQ10::END_RUN;
  item.s_runNumber = 1234;
  item.s_timeOffset = 456;
  item.s_Timestamp = now;
  strcpy(item.s_title, "This is a title");
  
  io::writeData(fd, &item, sizeof(item));
  
  // Create the ER info item and close the file:
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo10 er(fd);
  close(fd);                           // By now we're done
  
  
  // Anything involving body headers should throw std::string:
  
  
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

void EndInfo10Test::rangeThrows()
{
  // Open the temp file:
  
  char tmplate[] = "testrunXXXXXX";
  int fd = mkstemp(tmplate);
  
  // Create and write the first end run:
  
  time_t now = time(NULL);
  NSCLDAQ10::StateChangeItem item;
  item.s_header.s_size = sizeof(item);
  item.s_header.s_type = NSCLDAQ10::END_RUN;
  item.s_runNumber = 1234;
  item.s_timeOffset = 456;
  item.s_Timestamp = now;
  strcpy(item.s_title, "This is a title");
  
  io::writeData(fd, &item, sizeof(item));
  
  // Create the ER info item and close the file:
  
  lseek(fd, 0, SEEK_SET);
  CEndRunInfo10 er(fd);
  close(fd);                           // By now we're done

  // Attempting to get stuff for a non-existent end run buffer
  // throws std::range_error
  
  CPPUNIT_ASSERT_THROW(
    er.hasBodyHeader(1),
    std::range_error
  );
}