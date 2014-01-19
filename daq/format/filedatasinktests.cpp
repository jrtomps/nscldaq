#include <fstream>
#include <CPhysicsEventItem.h>
#include <cppunit/extensions/HelperMacros.h>

#include <ios>
#include <algorithm>
#include <CRingStateChangeItem.h>
#include <CRingItemFactory.h>
#include <CPhysicsEventItem.h>
#include <CRingScalerItem.h>
#include <CFileDataSource.h>
#include <CIStreamDataSource.h>
#include <RingItemUtils.h>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <errno.h>

#define private public

#include "CFileDataSink.h"


// A test suite 
class CFileDataSinkTest : public CppUnit::TestFixture
{
    private:
    CPhysicsEventItem m_item;

    public:
    CFileDataSinkTest();

    CPPUNIT_TEST_SUITE( CFileDataSinkTest );
//    CPPUNIT_TEST ( testReadOrThrow );
    CPPUNIT_TEST ( testConstructor1 );
    CPPUNIT_TEST ( testConstructor2 );
    CPPUNIT_TEST ( testConstructor3 );
    CPPUNIT_TEST ( testConstructor4 );
    CPPUNIT_TEST ( testPutItem );
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp();
    void tearDown();

    void testConstructor1();
    void testConstructor2();
    void testConstructor3();
    void testConstructor4();

    void testPutItem();

};



// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CFileDataSinkTest );

CFileDataSinkTest::CFileDataSinkTest()
  : m_item(8192)
{}


void CFileDataSinkTest::setUp()
{
    // create a ring item and fill it
    m_item = CPhysicsEventItem();    
    uint16_t* pCursor = reinterpret_cast<uint16_t*>(m_item.getBodyCursor());
    for (int i=0; i<10; i++) {
        *pCursor = i;
        pCursor++;
    }
    m_item.setBodyCursor(pCursor);
    m_item.updateSize();
}

void CFileDataSinkTest::tearDown()
{
}


//-------------------------------
// test stdout fd ctor
void CFileDataSinkTest::testConstructor1()
{
  CFileDataSink sink(STDOUT_FILENO);
  CPPUNIT_ASSERT( STDOUT_FILENO == sink.m_fd );    
} // test fd


//-------------------------------
// test stdout string ctor
void CFileDataSinkTest::testConstructor2()
{
  CFileDataSink sink("stdout");
  CPPUNIT_ASSERT( STDOUT_FILENO != sink.m_fd );    
} // end test string ctor


//-------------------------------
// test fails b/c not write only 
void CFileDataSinkTest::testConstructor3()
{
  int fd = open("./dummy",O_RDONLY|O_CREAT, S_IWUSR|S_IRUSR);
  if (fd<0) {
    std::cout << "after open...errno " << errno << std::endl;
  }
  CPPUNIT_ASSERT( fd >= 0 );

  bool caught = false;
  try {
    CFileDataSink sink(fd);
  } catch (std::string) {
    caught = true; 
  } 

  CPPUNIT_ASSERT(caught);

} // end test fail


//-------------------------------
// test closes fd
void CFileDataSinkTest::testConstructor4()
{
  int fd = -1;
  // Enclose the ctor in a block to make sure
  // the FileDataSink goes out of scope
  {
    try {
      CFileDataSink sink(std::string("dummy"));

      // reach in and store the fd
      fd = sink.m_fd;

    } catch (CException& err) {
      std::cout << err.ReasonText() << std::endl;
      CPPUNIT_ASSERT( false );
    }
    // this should go out of scope and close 
    // the file
  }
  // check that this is a bad file descriptor
  // when I try to work with it
  int status = fcntl(fd,F_GETFD);
  CPPUNIT_ASSERT( -1 == status  );
  CPPUNIT_ASSERT( EBADF == errno );
}

void CFileDataSinkTest::testPutItem()
{
    using namespace std;

    std::string fname = "testOutFile0.bin";
    {
      CFileDataSink sink(fname);
      sink.putItem(m_item);
      sink.flush();
    }

    std::ifstream ifile(fname.c_str(), ios::binary);
    CIStreamDataSource source(ifile);

    CRingItem* new_item = source.getItem();

    CPPUNIT_ASSERT( *m_item.getItemPointer() == *new_item->getItemPointer() );
}


