// Tests CTCPSocket as follows:
// Two Nodes are created.  One, a server
// listens for connections, blabs about them when
// they come in, and, for each message received on a server process socket,
// blabs to the terminal and echoes back to the client.
//
// 
// The second node, a client, delays, and then connects to the server,
// opens a file specified by argv[1] and loops as follows:
//    read a line from file,
//    send line to server.
//    read a message from server,
//    write message to stdout.  
//    
//  In order to test translations, the server will bind to the webcache port
//  while the client to 8080 (the port number for webcache).
//
#include <spectrodaq.h>
#include <CException.h>
#include <CErrnoException.h>
#include <CTCPConnectionLost.h>
#include <CSocket.h>
#include <iostream.h>
#include <string>
#include <CApplicationSerializer.h>
#include <stdio.h>
#include <string.h>

/*!
  Class comprising a client thread which plays with the server below.
  */
class Client : public DAQNode
{
  int operator()(int argc, char** argv);
};

int
Client::operator()(int argc, char** argv)
{
  CSocket sock;
  FILE*   pFile = NULL;
  string  filename(argv[1]);
  //  SetProcessTitle("CSocket test client");
  sleep(10);			// Wait for server to start up.
  cerr << "Talker is alive" << endl;
  cerr << "Will push " << filename << "to server\n";
  cerr.flush();
  try {
    sock.Connect("localhost", "8080");
    pFile = fopen(filename.c_str(), "r");
    if(!pFile) throw CErrnoException("Opening input file in client");
    try {
      while(!feof(pFile)) {
	char line[512];
	fgets(line, sizeof(line), pFile);	// Get a line from the file...
	sock.Write(line, strlen(line));	        // Write to socket.
	memset(line, 0, sizeof(line));
	sock.Read(line, sizeof(line)-1);        // Read back.
	cerr << "Client read: " << line << endl;
	cerr.flush();
      }
      fclose(pFile);
      sock.Shutdown();
    }
    catch (CTCPConnectionLost& rLost) {
      cerr << "Client connection lost " << rLost.ReasonText() << endl;
      cerr.flush();
      fclose(pFile);
      return 0;
    }
  }
  catch (CException& rExcept) {
    cerr << "Top level exception caught: " << rExcept.ReasonText() << endl;
    if(pFile) fclose(pFile);
    cerr.flush();
    return -1;
  }
  cerr << "end of file on " << filename << endl;
  cerr.flush();
  return 0;
}


/*!
  Class which comprises the thread which communicates with the client
  */
class ServerThread : public DAQThread {
  CSocket* m_pSocket;		//!< Socket connected to peer.
  int operator()(int argc, char** argv);

};

/*!
  Thread entry point.
  \param argc - number of params (1).
  \param argv - Arguments, just pointer to the comm. socket.
  */
int
ServerThread::operator()(int argc, char** argv)
{
  Detach();
  m_pSocket = (CSocket*)argv;
  short unsigned  port; 
  string peer;
  m_pSocket->getPeer(port, peer);
  string title = "Talking to: ";
  title += peer;
  SetThreadTitle(title.c_str());

  try {
    char buf[100];
    while(1) {
      memset(buf, 0, sizeof(buf));
      int nbytes = m_pSocket->Read(buf, sizeof(buf)-1);
      CApplicationSerializer::getInstance()->Lock();
      cerr << title << ": Got message: " << endl;
      cerr << buf << endl;
      CApplicationSerializer::getInstance()->UnLock();
      m_pSocket->Write(buf,nbytes);
    }
  }
  catch (CTCPConnectionLost& rLost) {
    cerr << title << "Lost connection" << endl;
    delete m_pSocket;
    return 0;
  }
  catch (CException& rOther) {
    cerr << title << "Caught an exception:" << endl;
    cerr << rOther.ReasonText() << endl;
    return rOther.ReasonCode();
  }
  
  return 0;
}

/*!
  Class which comprises the server listener.
*/
class Server : public DAQROCNode
{
  int operator()(int argc, char** argv);
};


int
Server::operator()(int argc, char** argv)
{
  CSocket Listner;
  Client talker;

  daq_dispatcher.Dispatch(talker, argc, argv);

  //  SetProcessTitle("webcache listener");

  try {
    Listner.Bind("webcache");
    Listner.Listen();
    while(1) {
      string client;
      CSocket* pServer = Listner.Accept(client);
      CApplicationSerializer::getInstance()->Lock();
      cerr << "Received a connection from " << client << endl;
      cerr.flush();
      ServerThread* pThread = new ServerThread;
      daq_dispatcher.Dispatch(*pThread, 1, (char**)pServer);
      CApplicationSerializer::getInstance()->UnLock();
    }
  }
  catch (CException& rExcept) {
    cerr << " Failed to set up listener: " ;
    cerr << rExcept.ReasonText() << endl;
    exit(-1);
  }
  return 0;
}


Server server;
