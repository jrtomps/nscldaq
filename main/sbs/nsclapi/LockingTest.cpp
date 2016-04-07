#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CVMEInterface.h>

#include <thread>
#include <iostream>
#include <chrono>

using namespace std;

// Simple RAII technique to make sure that we never fail
// to unlock the vme interface.
class ScopedVMELock {
  public:
  ScopedVMELock() {
    CVMEInterface::Lock();
  }

  ~ScopedVMELock() {
    CVMEInterface::Unlock();
  }
};

/*! \brief A set of tests to ensure that the CVMEInterface works as expected.
 *
 */
class LockingTest : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(LockingTest);
  CPPUNIT_TEST( tryLock_0 );
  CPPUNIT_TEST( tryLock_1 );
  CPPUNIT_TEST( tryLock_2 );
  CPPUNIT_TEST_SUITE_END();


  private:
  public:
  void setUp() {
  }
  void tearDown() {
  }
  protected:

  // this is a behavioral test. It merely record the default functionality that
  // has been tested to work.
  void tryLock_0 ()
  {
    //
    ScopedVMELock lock;

    // Because we are never trying to access the locked variable from more
    // than 1 thread at a time (note the join), we don't have to worry
    // about synchronization techniques
    bool locked;
    auto task = [&locked](){ 
      locked = CVMEInterface::TryLock(0); // timeout = 0 sec
    };

    // launch thread and let it finish
    std::thread t1(task);
    t1.join();

    EQMSG("TryLock should return false if it was unable to lock mutex",
        false, locked);
  }


  void tryLock_1 ()
  {
      bool locked = CVMEInterface::TryLock(0); // timeout = 0 sec

      CVMEInterface::Unlock();
      EQMSG("TryLock should return true if it was able to lock mutex",
            true, locked);
  }

void tryLock_2()
{
  using namespace std::chrono;
  ScopedVMELock lock;

    // Because we are never trying to access the locked variable from more
    // than 1 thread at a time (note the join), we don't have to worry
    // about synchronization techniques
    bool locked;
    time_point<high_resolution_clock> begin, end;
    auto task = [&locked, &begin, &end](){ 
      begin = high_resolution_clock::now();
      locked = CVMEInterface::TryLock(1); // timeout = 1 sec
      end = high_resolution_clock::now();
    };

    // launch thread and let it finish
    std::thread t1(task);
    t1.join();

    double waitTime = duration<double>(end-begin).count();

    EQMSG("TryLock should return false if it was unable to lock mutex",
        false, locked);
    ASSERTMSG("TryLock should take at least a second to fail",
        1.0 <= waitTime);
}





};

CPPUNIT_TEST_SUITE_REGISTRATION(LockingTest);
