TcpHoister is a tool which allows data to be received from a TCP/Ip link
and inserted into the NSCL Spectrodaq System.
Usage:

TcpHoister [-buffersize nbytes]  [-port portnum] hostname

Defaults:
    buffersize  8192 bytes
    port        daqhoist     - if defined in e.g. /etc/services
                2049         - if not defined.

