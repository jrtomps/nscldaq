//  Test server.. serves up a buffer.
//  when connected to.

#include <config.h>
#include <Iostream.h>


#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>  

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

void Server()
{
  // Make the socket:

  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if(sock == -1) {
    cerr << "Server - socket(2) failed\n";
    exit(errno);
  }
  // Bind it to port 2049

  struct sockaddr_in binding;
  binding.sin_family        = AF_INET;
  binding.sin_addr.s_addr   = htonl(INADDR_ANY);
  binding.sin_port          = htons(2049);

  if(bind(sock, (const sockaddr*)&binding, sizeof(binding)) < 0) {
    cerr << "Server bind(2) failed\n";
    exit(errno);
  }
  // Listen for a connection:

  if(listen(sock, 5) < 0) {
    cerr << "Server listen(2) failed\n";
    exit(errno);
  }
  // Accept the connection:

  struct sockaddr_in from;
  socklen_t          from_len;
  
  int comsocket = accept(sock, (sockaddr*)&from, &from_len);
  if(comsocket < 0) {
    cerr << "Server: accept(2) failed\n";
    exit(errno);
  }
  // Create the counting pattern buffer (which will look like phys data
  // coincidently,send it, shut everything down and exit.


  short buffer[4096];
  for(int i =0; i < 4096; i++) {
    buffer[i] = i;
  }
  if(send(comsocket, buffer, sizeof(buffer), 0) < 0 ) {
    cerr << "Server: send(2) failed\n";
    exit(errno);
  }

  shutdown(comsocket, 3);
  shutdown(sock, 3);


}

int main()
{
  Server();
  exit(0);
}

