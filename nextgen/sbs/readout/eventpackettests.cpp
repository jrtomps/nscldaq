// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#include "CEventPacket.h"
#include "CEventSegment.h"

#include <stdint.h>

// We need some test segments:

// This one indicates when a function has been delegated:

class CDelegateSegment : public CEventSegment
{
public:
  bool delegated;
  CDelegateSegment() : delegated(false) {}
  virtual void initialize() {delegated = true;}
  virtual void clear() {delegated = true;}
  virtual void disable() {delegated =true;}
  virtual size_t read(void* pBuffer, size_t maxwords) {delegated = true; return 0;}
};


class EVPacketTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EVPacketTest);
  CPPUNIT_TEST(delegates);
  CPPUNIT_TEST(encapsulation);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void delegates();
  void encapsulation();
};

class CountingPattern : public CEventSegment
{
public:
  virtual size_t read(void* pBuffer, size_t maxwords) {
    unsigned short* p = (unsigned short*)(pBuffer);
    for (int i =0; i < 10; i++) {
      *p++ = i;
    }
    return 10;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EVPacketTest);

//
// Checks that all functions that delegate do so:
//
void EVPacketTest::delegates() {
  CDelegateSegment seg;
  CEventPacket     packet(seg, 0x1234, "delegate", "Test delegation", "T1.0");

  unsigned short scratchpad[100];

  packet.initialize();
  ASSERT(seg.delegated);
  seg.delegated = false;
  packet.clear();
  ASSERT(seg.delegated);
  seg.delegated = false;
  packet.disable();
  ASSERT(seg.delegated);
  seg.delegated = false;
  packet.read(scratchpad, 100);
  ASSERT(seg.delegated);

}
//
//  Test for proper encapsulation
//
void
EVPacketTest::encapsulation()
{
  CountingPattern body;
  CEventPacket packet(body, 0xaaaa, "counter", "Counting pattern packet", "T1.0");

  struct event {
    uint32_t size;
    uint16_t type __attribute__((packed));
    uint16_t body[100] __attribute__((packed));
  } e;

  size_t nwords = packet.read(&e, sizeof(e)/sizeof(uint16_t));
  EQ(sizeof(uint32_t)/sizeof(uint16_t) + 11, nwords);
  for(uint16_t i=0; i < 10; i++) {
    EQ(i, e.body[i]);
  }
  EQ((uint16_t)0xaaaa, e.type);
  EQ(nwords, e.size);
}
