#ifndef __ASSERTS_H
#define __ASSERTS_H

#include <iostream>
#include <string>
class CTCLInterpreter;


// Abbreviations for assertions in cppunit.

#define EQMSG(msg, a, b)   CPPUNIT_ASSERT_EQUAL_MESSAGE(msg,a,b)
#define EQ(a,b)            CPPUNIT_ASSERT_EQUAL(a,b)
#define ASSERT(expr)       CPPUNIT_ASSERT(expr)
#define FAIL(msg)          CPPUNIT_FAIL(msg)

// Macro to test for exceptions:

#define EXCEPTION(operation, type) \
   {                               \
     bool ok = false;              \
     try {                         \
         operation;                 \
     }                             \
     catch (type e) {              \
       ok = true;                  \
     }                             \
     ASSERT(ok);                   \
   }

class Warning {

public:
  Warning(std::string message) {
    std::cerr << message << std::endl;
  }
};

extern std::string
getConfigVal(CTCLInterpreter& interp, const char* option, std::string configString);

#endif
