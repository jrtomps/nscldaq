#  ConnectionManager
#    This class contains code to manage connections by clients.
#    The client connections can request ports be allocated and that
#    port allocations be listed.  In addition the connection manager
#    senses when ports have been shut down.
# Member variables:
#    -ports            - A relation that allows the Connection manager to communicate with
#                       a PortManager object that is taking care of the actua port allocation.
#    -logger           - A relation that allows the Connection manager to make log file entries.
#    -listen           - The port on which we listen for client connections.
#    -usagefile        - File into which port usage will be written.
#    ClientInfo       - Information about each connected client:  This is an array indexed by connection
#                       channel. For each channel we keep the client's IP, the input so far, and
#                       the ports allocated to the client.
#    UsageFile        - The name of the usage file.
#
#  Of these members:
#     ListenPort defaults to 30000Ports
#     UsageFile defaults to /var/tmp/daqportmgr/ports.txt
# 

package require snit
package provide ConnectionManager 0.1

snit::type ConnectionManager {
    option -ports
    option -logger
    option -listen 30000
    option -usagefile  /var/tmp/daqportmgr/ports.txt
    
    variable ClientInfo
    variable ListenFd
    variable localhostIP 127.0.0.1
    
    #  Consructor:  The options are parsed.
    #   The /var/tmp/daqportmgr directory is created just in case.
    #   The listen is established with a connection callback on ListenPort
    #
    constructor {args} {
        catch {$self configurelist $args} msg
        
        #  We must have a Ports, and Logger member by now:
        
        if {$options(-ports) == ""} {
            error "Constructing a ConnectionManager must specify a PortManager via -ports"
        }
        if {$options(-logger) == ""} {
            error "Constructing a ConnectionManager must specify a Log via -logger"
        }
        #  Create the Usage file:
        
        file mkdir [file dirname $options(-usagefile)]
        
        $self writeUsageFile
        
        #  Now start listening on the Listener port..
        set ListenFd [socket -server [mymethod acceptConnection] $options(-listen)]
    }
    #  Destructor:
    #    Shutdown all sockets to clients
    #    Shutdown the listener
    
    destructor {
        #  Cancel fileevent and close sockets to all clients.
        
        foreach client [array names ClientInfo] {
            $self LostConnection $client
        }
        # Close the listener.

        catch {close $ListenFd} 
 
    }
    #  acceptConnection:
    #    Accepts a connection from a client:
    #      - The client is added to the ClientInfo array.
    #      - The connection is logged.
    #      - The communication channel is configured to line buffering and nonblocking.
    #      - A fileevent is registered for the channel so that when the client makes a request
    #        we can jump all over it.
    # Parameters:
    #   fd (channel):
    #       The channel that is open on the client.
    #   source (ip address):
    #       The client's ip in dotted notation.
    #   port (integer):
    #       The port number to which the communication (not listener) socket is bound).
    #   
    method acceptConnection {fd source port} {
        set ClientData [list $source "" ""]
        set ClientInfo($fd) $ClientData
        
        #  In logging the connection, try to use the name, not IP of the host:
        
        set PeerHost [GetPeer $fd]
        $options(-logger) logConnection $PeerHost
        
        #  Setup the communication channel for line buffering, nonblocking mode:
        
        fconfigure $fd -buffering line -blocking 0
        fileevent $fd readable [mymethod clientInput $fd]
        
    }
    # clientInput:
    #    Called when a client connection is readable.
    #    A client connection is readable if one
    #    of the following conditinos is true:
    #    - The client has dropped the connection: In which case we free their ports.
    #    - The client is sending us a request, in which case we accumulate the request
    #      and dispatch it when possible.
    #    I believe that line buffering will prevent us from being called with partial
    #    lines.  This works in test cases...
    # Parameters:
    #   fd (channel)
    #       The communications channel open to the peer.
    #
    method clientInput {fd} {

        
        set line [gets $fd]
        #
        #  Check for the client dropping out:
        #
        if {[eof $fd]} {
            if {[array names ClientInfo $fd] == $fd} {
                $self LostConnection $fd
            } else {
                close $fd
            }
            return
        }
        # Read the line from the socket, save and dispatch:


        set ClientData $ClientInfo($fd)
        set ClientInfo($fd) [lreplace $ClientData 1 1 $line]
        
        set request [lindex $line 0]
        switch -exact $request {
            GIMME {
                $self Gimme $fd [lindex $ClientData 0] [lindex $line 1] [lindex $line 2]
            }
            LIST {
                $self list $fd
            }
            default {
                # Log invalid requests and punish the client by
                # forcibly 'losing' the connection in case this is a DOS
                # in progress.
                #
                puts $fd [list FAIL "Invalid request $line"]
                $options(-logger) logInvalidRequest [GetPeer $fd]  $line
                $self LostConnection $fd
            }
        }
    }
    # Gimme:
    #   Client has requested a port allocation.
    #   - If the client was not local, logAllocation Fail and return FAIL to the
    #     caller.
    #   - Allocates a port from the port manager.
    #   - Reports ok back to the caller.
    #   - Logs the port allocatoin
    #   - Forces a re-write of the usage file.
    #   - 'bills' the port to the fd so that it can be released when the client exits.
    #
    # Parameters:
    #   fd  (channel):
    #       The channel open on the client.
    #   client (ip):
    #       The dotted numeric IP address of the host that is requesting the port.
    #   application (string):
    #       The name of the application that is requesting the port.
    #   user (string):
    #       The name of the user that is runing the application requesting the port.
    #
    method Gimme {fd client application user} {
        #
        #  The client must be the localhost:
        #
        if {$client != $localhostIP} {
            set Reason "[GetPeer $fd] : Only local applications get ports"
            $options(-logger) logAllocationFailure $application $user $Reason
            $self SendClient $fd [list FAIL $Reason]
            #
            # Force disconnect in case this is a DOS.
            #
            $self LostConnection $fd
            return
        }
        #  Allocate the port, log , bill and report back.
        
        if {[catch {$options(-ports) allocatePort $application $user} port]} {
            # Allocation failed:
            
            $options(-logger) logAllocationFailure $application $user $port
            $self SendClient $fd [list FAIL $port]
        } else {
            $options(-logger) logAllocation $port $application $user
            set clientinfo $ClientInfo($fd)
            set host       [lindex $clientinfo 0]
            set line       [lindex $clientinfo 1]
            set ports      [lindex $clientinfo 2]
            lappend ports [list $port $application $user]
            set ClientInfo($fd) [list $host $line $ports]
            $self writeUsageFile
            
            $self SendClient $fd [list OK $port]
        }
    }
    #  list:
    #    List the usage of the ports to the client. The port manager gives us the
    #    raw usage, but we need to uniquify the application names.  We sort by user
    #    then by application in our output to the user (different than the order
    #    gotten back from the port manager.
    #
    # Parameters:
    #   fd (channel):
    #       Channel open on the client.
    #
    method list {fd} {
        set usage [$options(-ports) listUsedPorts]
        set reply "OK [llength $usage]"
        
        #  The sort is done by throwing the ports into an array
        # indexed by [list $user $app] and then pulling them out
        # in sorted index order...lazy as heck but effective.
        
        foreach allocation $usage {
            set port [lindex $allocation 0]
            set app  [lindex $allocation 1]
            set user [lindex $allocation 2]
            
            lappend PortMap([list $user $app]) $port
        }
       foreach usage [lsort [array names PortMap]] {
            set user [lindex $usage 0]
            set app  [lindex $usage 1]
            set ports $PortMap($usage)
            set extension ""
            set instance 0
            foreach port $ports {
                set appname $app
                append appname $extension
                append reply "\n" [list $port $appname $user]
                incr instance
                set extension "_$instance"
            }
            
        }
        $self SendClient $fd $reply
    }
    # writeUsageFile:
    #    Writes the usage file.  This requires:
    #   - opening the file for w.
    #   - Asking the port manager to write the file.
    #   - Closing the file.
    #   NOTE:
    #     Inability to open the file for write will just result in
    #     no file being written.
    #
    method writeUsageFile {} {
        if {![catch {open $options(-usagefile) w} fd]} {
            $options(-ports) writeUsageFile $fd
            close $fd
        } else {
        }
    }
    #-------------------------------------------------------------------
    #
    # Private methods:
    
    # LostConnection fd:
    #    EOF encountered on client socket.
    #    - The port usage of the connection is fetched and then unset.
    #    - The ports held by the connection are released.
    #    - The fileevent on the fd is deleted.
    #    - The connection is closed.
    #
    method LostConnection {fd} {
        set fdinfo $ClientInfo($fd)
        unset ClientInfo($fd)
        
        set heldports [lindex $fdinfo 2]
        foreach usedport $heldports {
            set port [lindex $usedport 0]
            set app  [lindex $usedport 1]
            set user  [lindex $usedport 2]
            $options(-ports) freePort $port
            $options(-logger) logRelease $port $app $user
        }
        
        fileevent $fd readable ""
        close     $fd
        
        $self writeUsageFile
        
    }
    # SendClient fd msg:
    #   Send a message to a client.
    #   If the send fails, we assume that we lost the
    #   connection to the client and shut him down.
    #
    method SendClient {fd message} {
        if {[catch {puts $fd $message} error]} {
            $self LostConnection $fd
        }
    }
    #
    #  Figure out the hostname associated with a socket peer:
    #
    proc GetPeer {fd} {
        set hostinfo [fconfigure $fd -peername]
        return [lindex $hostinfo 1]
    }
}
