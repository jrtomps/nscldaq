#include <stdlib.h>
#include <stdio.h>

int main(int argc,char *argv[]) {
  // Keep a processor busy
  volatile long int v = 0;
  for(;;) v++;
}
