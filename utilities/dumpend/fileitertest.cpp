// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include <CFileIterator.h>

class FileIteratorTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FileIteratorTests);
  CPPUNIT_TEST(noFiles);
  CPPUNIT_TEST(oneFile);
  CPPUNIT_TEST(severalFiles);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void noFiles();
  void oneFile();
  void severalFiles();
};

class saveVisitor : public CFileIterator::CVisitor
{
public:
  std::vector<std::string> m_Names;
  
  void operator()(CFileIterator* p, std::string name) {
    m_Names.push_back(name);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FileIteratorTests);

void FileIteratorTests::noFiles() {
  CFileIterator i(0, NULL);
  saveVisitor v;
  i.foreach(v);
  
  EQ(size_t(0), v.m_Names.size());
}


void FileIteratorTests::oneFile()
{
  const char* files[] = {"test"};
  CFileIterator i(1, files);
  saveVisitor v;
  i.foreach(v);
  EQ(size_t(1), v.m_Names.size());
  EQ(std::string("test"), v.m_Names[0]);
  
}
void FileIteratorTests::severalFiles()
{
  const char* files[] = {"onefile", "twofile", "threefile", "more"};
  CFileIterator i(4, files);
  saveVisitor v;
  i.foreach(v);
  EQ(size_t(4), v.m_Names.size());
  
  for (int i =0; i < 4; i++) {
    EQ(std::string(files[i]), v.m_Names[i]);
  }
}