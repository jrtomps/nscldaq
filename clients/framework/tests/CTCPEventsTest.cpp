//  Test the TCP event classes.  Two classes are simultaneously
//  exercised:
//    CServerConnectionEvent - a class which listens for server connections
//                             and
//    CServerInstance        - Class which abstracts the behavior of simple
//                             server instances.
// NOTE:
//    This test leaves dangling socket servers around... an actual app
//    will probably want to have a joiner/deleter for them.
//

#include <spectrodaq.h>
#include <CServerConnectionEvent.h>
#include <CServerInstance.h>
#include <CTCPConnectionLost.h>
#include <iostream.h>
#include <string.h>

// This class manages the connection to a single client.

class MyInstance : public CServerInstance
{
  string m_Peer;
public:
  MyInstance(CSocket* pClient);
  virtual void OnRequest(CSocket* pClient);
};


// This class manages the server listener.

class MyListener : public CServerConnectionEvent
{
public:
  MyListener();
  virtual void OnConnection(CSocket* pPeer);
};

// This class represents the application's main thread:

class MyApp : public DAQROCNode
{
protected:
  virtual int operator()(int argc, char** argv);
};


// Implementation of the main program:
//
int
MyApp::operator()(int argc, char** argv)
{
  MyListener Server;

  Server.Enable();		// Start the server.

  char cmd[10];
  cin >> cmd;			// Block until stdin has data.

  return 0;			// Then exit.
}

MyApp app;

// Implementation of the server instance:

MyInstance::MyInstance(CSocket* pClient) :
  CServerInstance(pClient)
{
  unsigned short port;
  pClient->getPeer(port, m_Peer);
}

void
MyInstance::OnRequest(CSocket* pClient)
{
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));

  try {
    int nread = pClient->Read(buffer, sizeof(buffer) - 1);
    pClient->Write(buffer, nread);
  }
  catch (CTCPConnectionLost& rLost) {
    cerr << "Lost connection with peer " << m_Peer << endl;
    Shutdown();
  }
}

// Implementation of the server listener:

MyListener::MyListener() :
  CServerConnectionEvent("Listener", string("2048"))
{}

void
MyListener::OnConnection(CSocket* pPeer)
{
  string peer;
  unsigned short port;
  pPeer->getPeer(port, peer);
  cerr << "Got a connection request from " << peer << endl;
  cerr.flush();
  MyInstance* pInstance = new MyInstance(pPeer);
  pInstance->Enable();
}

