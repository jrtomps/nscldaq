// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMEList.h>
#include <CSimulatedVMEList.h>
#include <CSBSVMEInterface.h>
#include <CVMEPio.h>
#include <iostream>

#include <RangeError.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


class StartupMessage
{
public:
  StartupMessage(const char* message) {
    cerr << message << endl;
  }
};

StartupMessage msg1("The Simulated list tests require an SBS interface that");
StartupMessage msg2(" is connected to a VME crate with memory at 0x500000");
StartupMessage msg3("  If this condition is not met, tests will fail");

class TestSimAccess : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestSimAccess);
  CPPUNIT_TEST(countField);
  CPPUNIT_TEST(pattern);
  CPPUNIT_TEST(listCount);
  CPPUNIT_TEST(triggerCount);
  CPPUNIT_TEST(read32single);
  CPPUNIT_TEST(read16single);
  CPPUNIT_TEST(read8single);
  CPPUNIT_TEST(readTooBig);
  CPPUNIT_TEST(write32single);
  CPPUNIT_TEST(write16single);
  CPPUNIT_TEST(write8single);
  CPPUNIT_TEST(read32block);
  CPPUNIT_TEST(read16block);
  CPPUNIT_TEST(read8block);
  CPPUNIT_TEST(write32block);
  CPPUNIT_TEST(write32blockp);
  CPPUNIT_TEST(write16block);
  CPPUNIT_TEST(write16blockp);
  CPPUNIT_TEST(write8block);
  CPPUNIT_TEST(write8blockp);
  CPPUNIT_TEST(setCount);
  CPPUNIT_TEST(countfield32);
  CPPUNIT_TEST(countfield16);
  CPPUNIT_TEST(countfield8);
  CPPUNIT_TEST(hitread);
  CPPUNIT_TEST(cond32);
  CPPUNIT_TEST(cond16);
  CPPUNIT_TEST(cond8);
  CPPUNIT_TEST(nocond32);
  CPPUNIT_TEST(nocond16);
  CPPUNIT_TEST(nocond8);
  CPPUNIT_TEST(cond32block);
  CPPUNIT_TEST(cond16block);
  CPPUNIT_TEST(cond8block);
  CPPUNIT_TEST(nocond32block);
  CPPUNIT_TEST(nocond16block);
  CPPUNIT_TEST(nocond8block);
  CPPUNIT_TEST_SUITE_END();


private:
  CSBSVMEInterface*  m_pInterface;
  CVMEPio*           m_pIo;
  CSimulatedVMEList* m_pList;
  CVMEList*          m_pGenericList;
public:
  void setUp() {
    m_pInterface = new CSBSVMEInterface(0);
    m_pIo        = m_pInterface->createPioDevice();
    m_pList      = new CSimulatedVMEList(*m_pIo);
    m_pGenericList = m_pList;
  }
  void tearDown() {
    delete m_pList;
    delete m_pIo;
    delete m_pInterface;
  }
protected:
  void countField();
  void pattern();
  void listCount();
  void triggerCount();

  void read32single();
  void read16single();
  void read8single();
  void readTooBig();

  void write32single();
  void write16single();
  void write8single();

  void read32block();
  void read16block();
  void read8block();

  void write32block();
  void write32blockp();
  void write16block();
  void write16blockp();
  void write8block();
  void write8blockp();
  void setCount();
  void countfield32();
  void countfield16();
  void countfield8();

  void hitread();
  void cond32();
  void cond16();
  void cond8();
  void nocond32();
  void nocond16();
  void nocond8();
  void cond32block();
  void cond16block();
  void cond8block();
  void nocond32block();
  void nocond16block();
  void nocond8block();
};


CPPUNIT_TEST_SUITE_REGISTRATION(TestSimAccess);

void TestSimAccess::countField() {
  uint8_t shift = (uint8_t)3;
  uint32_t mask = 0xfe;

  m_pList->setCountExtractionParameters(shift, mask);
  EQMSG("shift", shift, m_pList->getCountRightShift());
  EQMSG("mask",  mask,  m_pList->getCountMask());
}

void TestSimAccess::pattern()
{
  uint16_t mask = 0xaaaa;
  m_pList->setConditionMask(mask);
  EQ(mask, m_pList->getConditionMask());
}

void TestSimAccess::listCount()
{
  EQ((size_t)0, m_pList->listCount());
}
void TestSimAccess::triggerCount()
{
  EQ((size_t)0, m_pList->triggerCount());
}

// Single shot reads:

void TestSimAccess::read32single()
{
  // Set 0x500000 to 0x12345678, add a single shot long read
  // for that and see what we get when its executed:

  m_pIo->write32(0x39, 0x500000, 0x12345678);
  m_pList->addRead32(0x39, 0x500000);

  uint32_t  buffer;
  size_t    nread = m_pList->execute(&buffer, sizeof(uint32_t));
  EQMSG("return size", sizeof(uint32_t), nread);
  EQMSG("data", (uint32_t)0x12345678, buffer);
}
void TestSimAccess::read16single()
{
  // Similar to read32single, but only a
  // 16 bit pattern used.
  m_pIo->write16(0x39, 0x500000, 0xaaaa);
  m_pList->addRead16(0x39, 0x500000);

  uint16_t  buffer;
  size_t    nread = m_pList->execute(&buffer, sizeof(uint16_t));
  EQMSG("return size", sizeof(uint16_t), nread);
  EQMSG("data", (uint16_t)0xaaaa, buffer);

}
void TestSimAccess::read8single()
{
  // same as the others, but read an 8 bit.

  m_pIo->write8(0x39, 0x500000, 0xff);
  m_pList->addRead8(0x39, 0x500000);

  uint8_t  buffer;
  size_t    nread = m_pList->execute(&buffer, sizeof(uint8_t));
  EQMSG("return size", sizeof(uint8_t), nread);
  EQMSG("data", (uint8_t)0xff, buffer);
}
void TestSimAccess::readTooBig()
{
  // See if we get the right exception when we fall off the end ofthe
  // buffer.  NOte that a normal test would be unsafe, so we are going
  // to allocate a buffer big enough to hold the data but
  // lie about its size to the list processor.

  m_pIo->write32(0x39, 0x500000, 0x12345678);
  m_pList->addRead32(0x39, 0x500000);

  uint32_t  buffer;
  bool thrown = false;
  try {
    size_t    nread = m_pList->execute(&buffer, sizeof(uint16_t)); // too small!
  }
  catch(CRangeError &r) {
    thrown = true;
  }
  ASSERT(thrown);
  
}


void TestSimAccess::write32single()
{
  m_pList->addWrite32(0x39, 0x500000, 0xaaaaaaaa);
  m_pList->execute(0, 0);	// Should not need a buffer.
  EQ((unsigned long)0xaaaaaaaa, m_pIo->read32(0x39, 0x500000));
}
void TestSimAccess::write16single()
{
  m_pList->addWrite16(0x39, 0x500000, 0x5555);
  m_pList->execute(0, 0);
  EQ((unsigned short)0x5555, m_pIo->read16(0x39, 0x500000));
}
void TestSimAccess::write8single()
{
  m_pList->addWrite8(0x39, 0x500000, 0xff);
  m_pList->execute(0,0);
  EQ((unsigned char)0xff, m_pIo->read8(0x39, 0x500000));
}


void TestSimAccess::read32block()
{
  // Fill in a bunch of words and then block read them:

  unsigned long base = 0x500000ul;
  for(int i=0; i < 0x1000; i++) {
    m_pIo->write32(0x39, base + i*sizeof(uint32_t), i);
  }
  m_pList->addBlockRead32(0x39, 0x500000, 0x1000);
  uint32_t buffer[0x2000];
  m_pList->execute(buffer, sizeof(buffer));

  for (int i =0; i < 0x1000; i++) {
    EQ((uint32_t)i, buffer[i]);
  }

}

void TestSimAccess::read16block()
{
  // Fill in a bunch of words and then block read them:

  unsigned long base = 0x500000ul;
  for(int i=0; i < 0x1000; i++) {
    m_pIo->write16(0x39, base + i*sizeof(uint16_t), (i%1) ? 0xaaaa : 0x5555);
  }
  m_pList->addBlockRead16(0x39, 0x500000, 0x1000);
  uint16_t buffer[0x2000];
  m_pList->execute(buffer, sizeof(buffer));

  for (int i =0; i < 0x1000; i++) {
    EQ((uint16_t)((i%1) ? 0xaaaa : 0x5555), buffer[i]);
  }

}
void TestSimAccess::read8block()
{
  // Fill in a bunch of words and then block read them:

  unsigned long base = 0x500000ul;
  for(int i=0; i < 0x1000; i++) {
    m_pIo->write8(0x39, base + i, 0xff);
  }
  m_pList->addBlockRead8(0x39, 0x500000, 0x1000);
  uint8_t buffer[0x2000];
  m_pList->execute(buffer, sizeof(buffer));

  for (int i =0; i < 0x1000; i++) {
    EQ((uint8_t)0xff, buffer[i]);
  }

}

void TestSimAccess::write32block()
{
  vector<uint32_t> data;
  for (int i=0; i < 100; i++) {
    data.push_back(i);
  }
  m_pList->addBlockWrite32(0x39, 0x500000, data);
  m_pList->execute(0,0);	// Should not need input buffer.

  for(int i =0; i < 100; i++) {
    EQ(data[i], (uint32_t)m_pIo->read32(0x39, 0x500000 + i*sizeof(uint32_t)));
  }

}
void TestSimAccess::write32blockp()
{
  uint32_t data[100];
  for (int i =0; i < 100; i++) {
    data[i] = (i%1) ? 0xaaaaaaaa : 0x55555555;
  }
  m_pGenericList->addBlockWrite32(0x39, 0x500000, (void*)data, (size_t)100);
  m_pGenericList->execute(0,0);

  for(int i=0; i < 100; i++) {
    EQ(data[i], (uint32_t)m_pIo->read32(0x39, 0x500000 + i*sizeof(uint32_t)));
  }
}

void TestSimAccess::write16block()
{
  vector<uint16_t> data;
  for (int i=0; i < 100; i++) {
    data.push_back(i);
  }
  m_pList->addBlockWrite16(0x39, 0x500000, data);
  m_pList->execute(0,0);	// Should not need input buffer.

  for(int i =0; i < 100; i++) {
    EQ(data[i], (uint16_t)m_pIo->read16(0x39, 0x500000 + i*sizeof(uint16_t)));
  }
}
void TestSimAccess::write16blockp()
{
  uint16_t data[100];
  for (int i =0; i < 100; i++) {
    data[i] = (i%1) ? 0xaaaa : 0x5555;
  }
  m_pGenericList->addBlockWrite16(0x39, 0x500000, (void*)data, (size_t)100);
  m_pGenericList->execute(0,0);

  for(int i=0; i < 100; i++) {
    EQ(data[i], (uint16_t)m_pIo->read16(0x39, 0x500000 + i*sizeof(uint16_t)));
  }
}
void TestSimAccess::write8block()
{
  vector<uint8_t> data;
  for (int i=0; i < 100; i++) {
    data.push_back(i);
  }
  m_pList->addBlockWrite8(0x39, 0x500000, data);
  m_pList->execute(0,0);	// Should not need input buffer.

  for(int i =0; i < 100; i++) {
    EQ(data[i], (uint8_t)m_pIo->read8(0x39, 0x500000 + i*sizeof(uint8_t)));
  }
}
void TestSimAccess::write8blockp()
{
  uint8_t data[100];
  for (int i =0; i < 100; i++) {
    data[i] = (i%1) ? 0xaa : 0x55;
  }
  m_pGenericList->addBlockWrite8(0x39, 0x500000, (void*)data, (size_t)100);
  m_pGenericList->execute(0,0);

  for(int i=0; i < 100; i++) {
    EQ(data[i], (uint8_t)m_pIo->read8(0x39, 0x500000 + i*sizeof(uint8_t)));
  }
}

void TestSimAccess::setCount()
{
  m_pGenericList->defineCountField(3, 0x0f0);
  m_pGenericList->execute(0,0);

  EQMSG("shift", (uint8_t)3, m_pList->getCountRightShift());
  EQMSG("mask",  (uint32_t)0x0f0, m_pList->getCountMask());
}

void TestSimAccess::countfield32()
{
  // First need to write a pattern to memory that has a count field
  // our count field will be in the bits 4-12.
  // and will be 100(10).
  //
  int32_t datum = 100 << 4 | 0x1234000f; // add in some spicing bits too.
  m_pIo->write32(0x39, 0x500000, datum);
  for(int i =0; i < 100; i++) {
    m_pIo->write32(0x39, 0x500000 + (i+1)*sizeof(int32_t), i);
  }
  m_pGenericList->defineCountField(4, 0xff);
  m_pGenericList->addCountFieldRead32(0x39, 0x500000);
  uint32_t buffer[200];
  m_pGenericList->execute(buffer, sizeof(buffer));

  EQMSG("count field", datum, (int32_t)buffer[0]);
  for (int i=1; i <=100; i++) {
    EQMSG("data", (uint32_t)(i-1), buffer[i]);
  }
  
}
void TestSimAccess::countfield16()
{
  // The idea is the same as countfield32 but using 16 bit operations.
  
  int16_t datum = (100 << 4) | 0xf00f;
  m_pIo->write16(0x39, 0x500000, datum);
  for (int i=0; i < 100; i++) {
    m_pIo->write16(0x39, 0x500000+(i+1)*sizeof(uint16_t),
		   (i % 1) ? 0xaaaa : 0xffff);
  }
  m_pGenericList->defineCountField(4, 0xff);
  m_pGenericList->addCountFieldRead16(0x39, 0x500000);

  uint16_t buffer[200];
  m_pGenericList->execute(buffer, sizeof(buffer));

  EQMSG("count field", (uint16_t)datum, buffer[0]);
  for (int i=0; i < 100; i++) {
    EQMSG("data", (uint16_t)((i % 1) ? 0xaaaa : 0xffff), buffer[i+1]);
  }
 

}
void TestSimAccess::countfield8()
{
  // Same idea as the 32 bit test but with bytes.

  int8_t datum = (12 << 2) | 0x81;
  m_pIo->write8(0x39, 0x500000, datum);
  for (int i=0; i < 12; i++) {
    m_pIo->write8(0x39, 0x500000+i+1, 'a'+i);
  }
  m_pGenericList->defineCountField(2, 0xf);
  m_pGenericList->addCountFieldRead8(0x39, 0x500000);

  int8_t buffer[256];
  m_pGenericList->execute(buffer, sizeof(buffer));

  EQMSG("count field", datum, buffer[0]);
  for (int i =0; i < 12; i++) {
    EQMSG("data", (int8_t)('a'+i), buffer[i+1]);
  }
}

void TestSimAccess::hitread()
{
  // place a value in memory, read it as the hit register and
  // be sure that's what we got...
  
  m_pIo->write16(0x39, 0x500000, 0xaa55);
  m_pGenericList->addHitRegisterRead(0x39, 0x500000);
  unsigned short buffer;

  m_pGenericList->execute(&buffer,sizeof(buffer));

  EQ((unsigned short)0xaa55, m_pList->getConditionMask());
}

// Conditional single read tests when the condition is made.
// We test the version with the terms specified as C arrays as
// this tests the marshalling to the vector call as well as the vector call itself.
void TestSimAccess::cond32()
{
  m_pList->setConditionMask(0xaaaa); // This is the condition.
  uint16_t terms[2] = {1, 2};        // Second term makes it.
  m_pIo->write32(0x39, 0x500000, 0x12345678);

  m_pGenericList->addConditionalRead32(terms, 2, 0x39, 0x500000);
  
  int32_t buffer(0);
  m_pGenericList->execute(&buffer, sizeof(buffer));
  EQ((int32_t)0x12345678, buffer);
  
}
void TestSimAccess::cond16()
{
  // Same as cond32 but 16 bit reads.

  m_pList->setConditionMask(0x5555); // This is the condition.
  uint16_t terms[2] = {1, 2};        // first term makes it.
  m_pIo->write16(0x39, 0x500000, 0xffff);

  m_pGenericList->addConditionalRead16(terms, 2, 0x39, 0x500000);
  
  int16_t buffer(0);
  m_pGenericList->execute(&buffer, sizeof(buffer));
  EQ((int16_t)0xffff, buffer);
}
void TestSimAccess::cond8()
{

  // Same as cond32 but 8 bit reads.

  m_pList->setConditionMask(0x2); // This is the condition.
  uint16_t terms[2] = {1, 2};        // Second term makes it.
  m_pIo->write8(0x39, 0x500000, 0xaa);

  m_pGenericList->addConditionalRead8(terms, 2, 0x39, 0x500000);
  
  int8_t buffer(0);
  m_pGenericList->execute(&buffer, sizeof(buffer));
  EQ((int8_t)0xaa, buffer);
}
// Conditional single read tests when the conditional is not made.

void TestSimAccess::nocond32()
{
  m_pList->setConditionMask(0xaaaa);
  uint16_t terms[2] = {0x1, 0x4}; // Neither term is good.
  m_pGenericList->addConditionalRead32(terms, 2, 0x39, 0x500000);
  int32_t buffer;
  size_t after =  m_pGenericList->execute(&buffer, sizeof(buffer));
  EQ((size_t)0, after);

}
void TestSimAccess::nocond16()
{
  m_pList->setConditionMask(0xaaaa);
  uint16_t terms[2] = {0x1, 0x4}; // Neither term is good.
  m_pGenericList->addConditionalRead16(terms, 2, 0x39, 0x500000);
  int32_t buffer;
  size_t after = m_pGenericList->execute(&buffer, sizeof(buffer));
  EQ((size_t)0, after);
}
void TestSimAccess::nocond8()
{
  m_pList->setConditionMask(0xaaaa);
  uint16_t terms[2] = {0x1, 0x4}; // Neither term is good.
  m_pGenericList->addConditionalRead8(terms, 2, 0x39, 0x500000);
  int32_t buffer;
  size_t after = m_pGenericList->execute(&buffer, sizeof(buffer));
  EQ((size_t)0, after);
}

// Block mode conditional reads where the condition is made...

void TestSimAccess::cond32block()
{
  m_pList->setConditionMask(0xaaaa);
  uint16_t terms[2] = {0x1, 0x2};
  m_pGenericList->addConditionalBlockRead32(terms, 2, 0x39, 0x500000,
					    100);

  // set the data:

  for (int i=0; i < 100; i++) {
    m_pIo->write32(0x39, 0x500000+i*sizeof(uint32_t), i);
  }

  int32_t buffer[200];
  size_t numRead = m_pGenericList->execute(buffer, sizeof(buffer));
  EQMSG("transfer count", (size_t)(100*sizeof(int32_t)), numRead);

  for (int i = 0; i < 100; i++) {
    EQMSG("data", (int32_t)i, buffer[i]);
  }

}
void TestSimAccess::cond16block()
{
  m_pList->setConditionMask(0xaaaa);
  uint16_t terms[2] = {0x1, 0x2};
  m_pGenericList->addConditionalBlockRead16(terms, 2, 0x39, 0x500000,
					    100);

  // set the data:

  for (int i=0; i < 100; i++) {
    m_pIo->write16(0x39, 0x500000+i*sizeof(uint16_t), (i%1) ? 0x5555 : 0xaaaa);
  }

  int16_t buffer[200];
  size_t numRead = m_pGenericList->execute(buffer, sizeof(buffer));
  EQMSG("transfer count", (size_t)(100*sizeof(int16_t)), numRead);

  for (int i = 0; i < 100; i++) {
    EQMSG("data", (int16_t)((i % 1) ? 0x5555 : 0xaaaa), buffer[i]);
  }
}
void TestSimAccess::cond8block()
{
  m_pList->setConditionMask(0xaaaa);
  uint16_t terms[2] = {0x1, 0x2};
  m_pGenericList->addConditionalBlockRead8(terms, 2, 0x39, 0x500000,
					    100);

  // set the data:

  for (int i=0; i < 100; i++) {
    m_pIo->write8(0x39, 0x500000+i*sizeof(uint8_t), (i%1) ? 0xff : 0x0);
  }

  int8_t buffer[200];
  size_t numRead = m_pGenericList->execute(buffer, sizeof(buffer));
  EQMSG("transfer count", (size_t)(100*sizeof(int8_t)), numRead);

  for (int i = 0; i < 100; i++) {
    EQMSG("data", (int8_t)((i % 1) ? 0xff : 0x0), buffer[i]);
  }
}

// Conditional block reads that fail the condition:

void TestSimAccess::nocond32block()
{
  m_pList->setConditionMask(0x5555);
  uint16_t terms[2] = {0x2, 0x8};
  m_pGenericList->addConditionalBlockRead32(terms, 2, 0x39, 0x500000, 100);

  int32_t buffer[200];
  size_t numRead = m_pGenericList->execute(buffer, sizeof(buffer));
  EQ((size_t)0, numRead);

}
void TestSimAccess::nocond16block()
{
  m_pList->setConditionMask(0x5555);
  uint16_t terms[2] = {0x2, 0x8};
  m_pGenericList->addConditionalBlockRead16(terms, 2, 0x39, 0x500000, 100);

  int32_t buffer[200];
  size_t numRead = m_pGenericList->execute(buffer, sizeof(buffer));
  EQ((size_t)0, numRead);

}
void TestSimAccess::nocond8block()
{
  m_pList->setConditionMask(0x5555);
  uint16_t terms[2] = {0x2, 0x8};
  m_pGenericList->addConditionalBlockRead8(terms, 2, 0x39, 0x500000, 100);

  int32_t buffer[200];
  size_t numRead = m_pGenericList->execute(buffer, sizeof(buffer));
  EQ((size_t)0, numRead);

}

// block mode conditional reads where the condition is not satisfied.
