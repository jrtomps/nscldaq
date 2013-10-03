//
//  This program tests the port manager access library.
//  The assumption is that the port manager is already
//  running on the default port.
//  If this is not the case, the test will fail.
//    
#include <config.h>
#include "CPortManager.h"
#include "CPortManagerException.h"
#include <iostream>
#include <vector>
#include <stdlib.h>


using namespace std;

/*
   test1: 
     Construct a port manager on port 1234 (wrong port) for localhost.
     Attempt to allocate a port. Should get a ConnectionFailed.
*/
int
test1()
{
  CPortManager pm("localhost", 1234);
  bool threw=false;
  cerr << "test1 ...";
  try { 
    int port = pm.allocatePort("TestApp");
    cerr << " no exception thrown";
  }
  catch (CPortManagerException& ex) {
    if((CPortManagerException::Reason)(ex.ReasonCode()) == 
       CPortManagerException::ConnectionFailed) {
      threw = true;
    } else {
      cerr << " caught right exception, wrong reason";
      cerr << ex.ReasonCodeToText(ex.ReasonCode());
    }
  }
  catch(...) {
    cerr << "Wrong exception type";
  }
  if (threw) {
    cerr << "passesd\n";
    return 0;
  } else {
    cerr << " failed";
    return 1;
  }
}

/*
   Test port allocation from a good port..
  We can't pre-determine the port we should
  get  we just require no exception.
*/
int
test2()
{
  bool threw = false;
  cerr << "test2...";
  CPortManager pm ("localhost");
  try {
    int port = pm.allocatePort("testapp");
    cerr << "Got: " << port;
  }
  catch (CPortManagerException& e) {
    cerr << "CPortManagerException" << e;
    threw = true;
  }
  catch(...) {
    cerr << "other exception";
    threw = true;
  }
  if (threw) {
    cerr << "failed\n";
    return 1;
  } else {
    cerr << "passed\n";
    return 0;
  }

}
/*
   Get two ports.. they should be different.
*/
int
test3()
{
  cerr << "test3...";

  CPortManager pm ("localhost");
  int port1 = pm.allocatePort("test");
  int port2 = pm.allocatePort("test2");
  if(port1 != port2) {
    cerr << "passed\n";
    return 0;
  }
  else {
    cerr << "Ports the same: " << port1 << " " << port2
	 << " failed\n";
    return 1;
  }


}
/*  
   List the ports in use... should be 3 in use.
*/
int
test4()
{
  cerr << "test4...";
  CPortManager pm("localhost");
  vector<CPortManager::portInfo> info;
  try {
    info = pm.getPortUsage();
  } 
  catch (CPortManagerException e) {
    cerr << "Caught CPortmanagerException: " << e;
    cerr << "failed\n";
    return 1;
  }
  catch (...) {
    cerr << "Caught some other exception failed\n";
    return 1;
  }


  if(info.size() ==1) {
    cerr << "passed\n";
    return 0;
  }
  else {
    cerr << "Incorrect # of port information items: " 
	 << info.size() << " failed\n";
    return 1;
  }
}

int main()
{
  cerr << "Testing the port manager C++ interface\n";
  int failures = test1()
    + test2()
    + test3();
  //  test4();   // depends too much on testing environment.c3

  exit(failures);
}

