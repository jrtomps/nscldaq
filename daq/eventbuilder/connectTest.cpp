// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <errno.h>

#include <CPortManager.h>
#include "CEventOrderClient.h"
#include <ErrnoException.h>
#include <CSocket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <list>

class connectTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(connectTests);
  CPPUNIT_TEST(noServer);
  CPPUNIT_TEST(serverOk);
  CPPUNIT_TEST(serverError);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void noServer();
  void serverOk();
  void serverError();
};

CPPUNIT_TEST_SUITE_REGISTRATION(connectTests);

//
// With no server present, the connect should fail with a 
// ECONNREFUSED
//
void connectTests::noServer() {
  bool thrown = false;
  bool  rightException = false;
  std::list<int> types;
  try {
    CEventOrderClient client("localhost", 11223);
    client.Connect("TestClient", types);
  }
  catch (CErrnoException& e) {
    thrown = true;
    rightException = e.ReasonCode() == ECONNREFUSED;
  }
  catch (...) {
    thrown = true;
  }
  ASSERT(thrown);
  ASSERT(rightException);
}
//
// With the server present, the connection should receive a CONNECT
// message with the description as a payload.
// To that we'll reply OK and should have everything just fine.
//
void connectTests::serverOk() {
  char portString[32];


  // Set up the server socket...

  // Fork the client off.  It will just connect and we'll expect to get the
  // connection message

  pid_t pid = fork();
  int p;
  if (pid) {			// parent
    CPortManager manager;
     p = manager.allocatePort("ORDERER");
     sprintf(portString, "%u", p);
     std::string Port(portString);    std::string client;
     
     CSocket* server = new CSocket;;
     server->Bind(Port);
     server->Listen();		// Now connect should be honored.
     server->setLinger(false, 0);
     
     
     CSocket* pClient = server->Accept(client);
     EQ(std::string("localhost"), client);
     
     // CONNECT string:
     
     uint32_t stringSize;
     EQ((int)sizeof(uint32_t), pClient->Read(&stringSize, sizeof(uint32_t)));
     EQ(strlen("CONNECT"), stringSize);
     char request[stringSize+1];
     memset(request, 0, stringSize+1);
     
     EQ((int)strlen("CONNECT"), pClient->Read(request, stringSize));
     EQ(std::string("CONNECT"), std::string(request));
     
     // "test connecdtion" string
     
     EQ((int)sizeof(uint32_t), pClient->Read(&stringSize, sizeof(uint32_t)));
     EQ(strlen("test connection"), stringSize);
     
     char body[stringSize+1];
     memset(body, 0, stringSize+1);
     EQ((int)strlen("test connection"), pClient->Read(body, stringSize));
     EQ(std::string("test connection"), std::string(body));
     
     // Reply with "OK"
     
     std::string reply("OK\n");
     pClient->Write(reply.c_str(), reply.size());
     
     // Reap the child.
     
     int status;
    pid_t endpid = wait(&status);
    EQ(pid, endpid);
    delete server;
    pClient->Shutdown();
    delete pClient;

    
  } else {			// child
    sleep(2);
    try {
      std::list<int> types;
      CEventOrderClient client("localhost", CEventOrderClient::Lookup("localhost"));
      client.Connect("test connection", types);
    }
    catch (...) {}
    exit(0);
  }
  

}

//
// With the server returning an error rather than OK, Connect should throw an
// ECONNREFUSED Errno exception.

void
connectTests::serverError()
{

  char portString[32];

  // Set up the server socket...


  // Fork the client off.  It will just connect and we'll expect to get the
  // connection message

  pid_t pid = fork();

  if (!pid) {			// child
    try {
    CPortManager manager;
   
    int p = manager.allocatePort("ORDERER");
    sprintf(portString, "%u", p);
    std::string Port(portString);
    CSocket* server = new CSocket;
    try {
    server->Bind(Port);
    server->Listen();		// Now connect should be honored.
    server->setLinger(false, 0);
    } 
    catch(CException& e) {
    exit(-1);
    }
    std::string client;
    CSocket* pClient = server->Accept(client);

    // CONNECT string:

    uint32_t stringSize;
    pClient->Read(&stringSize, sizeof(uint32_t));
    char request[stringSize+1];
    memset(request, 0, stringSize+1);


    pClient->Read(request, stringSize);
    
    // "test connection" string
    pClient->Read(&stringSize, sizeof(uint32_t));

    
    char body[stringSize+1];
    memset(body, 0, stringSize+1);
    pClient->Read(body, stringSize);
     
    // Reply with "ERROR -1

    std::string reply("ERROR -1\n");
    pClient->Write(reply.c_str(), strlen(reply.c_str()));
    delete server;
    }
    catch (...) {
    }
    exit(0);
    
  } else {			// parent
    sleep(3);			// let child start server.
    bool threw = false;
    bool rightError = false;
    CEventOrderClient client("localhost", CEventOrderClient::Lookup("localhost"));
     
    try {
      std::list<int> types;
      client.Connect("test connection", types);
    }
    catch (CErrnoException& e) {
      threw = true;
      rightError = e.ReasonCode() == ECONNREFUSED;
    }
    catch (...) {
      threw = true;
    }
    ASSERT(threw);
    ASSERT(rightError);
    // Reap the child.
    
    int status;
    pid_t endpid = wait(&status);
    EQ(pid, endpid);

  }

}
