

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CFileDataSource.h"
#include "ByteBuffer.h"

#include <fstream>
#include <string>
#include <algorithm>

using namespace std;
using namespace DAQ;

// A test suite
class CFileDataSourceTest : public CppUnit::TestFixture
{
    private:
    std::string m_testPath;

    public:
    CPPUNIT_TEST_SUITE( CFileDataSourceTest );
    CPPUNIT_TEST(peek_0);
    CPPUNIT_TEST(availableData_0);
    CPPUNIT_TEST(availableData_1);
    CPPUNIT_TEST(ignore_0);
    CPPUNIT_TEST(ignore_1);
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp() {

        Buffer::ByteBuffer item;
        item << uint32_t(24);
        item << uint32_t(30);
        item << uint64_t(0x123456789);
        item << uint32_t(1);
        item << uint32_t(214);

        m_testPath = "___---test---___.evt";
        std::ofstream tempfile(m_testPath.c_str());
        tempfile.write(reinterpret_cast<char*>(item.data()), item.size());
    }

    void tearDown() {
        std::remove(m_testPath.c_str());
    }

    void peek_0() {
        char data[4];

        CFileDataSource file(m_testPath);
        size_t nRead = file.peek(data, sizeof(data));

        size_t pos = file.tell();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("peek returns expected number of bytes",
                               size_t(4), nRead);

        char expected[] = {24, 0, 0, 0};
        CPPUNIT_ASSERT_MESSAGE("peek returns expected data",
                               equal(expected, expected+sizeof(expected), data));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("peek does not move the file position",
                                     size_t(0), pos);

    }


    void availableData_0() {
        char data[4];

        CFileDataSource file(m_testPath);
        size_t nBytes = file.availableData();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("available data should be entire file prior to any reads",
                                     size_t(24), nBytes);

    }

    void availableData_1() {
        char data[4];

        CFileDataSource file(m_testPath);
        file.read(data, sizeof(data));
        size_t nBytes = file.availableData();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("available data should be entire file prior to any reads",
                                     size_t(20), nBytes);

    }

    void ignore_0() {

        CFileDataSource file(m_testPath);
        file.ignore(10);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("offset should change after ignore",
                                     size_t(10), file.tell());

    }

    void ignore_1() {
        union {
            uint32_t value;
            char     bytes[sizeof(uint32_t)];
        } hybrid;

        CFileDataSource file(m_testPath);
        file.ignore(20);

        file.read(hybrid.bytes, sizeof(uint32_t));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("offset should change after ignore",
                                     uint32_t(214), hybrid.value);

    }


};



// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CFileDataSourceTest );
