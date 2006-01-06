#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#


#   This package provides data sources that read NSCL
#   DAQ event files.  Two sources are supplied.
#   offlineDataSource  - provides access to an event file
#                        Buffers read from the event file are
#                        turned into Tcl lists.
#   onlineDataSource   - Provides access to an online data source.
#                        Buffers read from the online source are
#                        turned into Tcl lists.  Note that any
#                        buffere produced by the online source
#                        the application is unable to keep up with
#                        are thrown on the floor.
#   dataSource         - A generic data source that creates and
#                        opens one of the above.  This fulfills the
#                        role of a data source factory actually,
#                        and is the class that most users will create.
#

package provide dataSources 1.0
package require snit
package require bufferAssembly
package require dns

# dns has logger debugging on by default.. turn off
# that spew:

catch {logger::disable debug}

#------------------------------------------------------------------------------
# offlineDataSource
#     Provides access to an event file.  Buffers read from the event file
#     are turned into lists by piping them through the hexdump program.
# Options:
#    -file        Specifies the name of the data source.
#    -buffersize  Specifies the size of event buffers read from the data source
#                 in bytes.
#
snit::type offlineDataSource {
    option -file         {}
    option -buffersize 8192

    variable fd          {}
    variable partial     {}

    constructor args {
        $self configurelist $args
    }
    destructor {
        $self close
    }
    # open
    #   Opens the data source
    # Implicit inputs:
    #    The filename comes from $options(-file)
    # Errors:
    #    {File not Readable}  - No permissions to read the file.
    method open {} {
        set name $options(-file)

        if {![file exists $name]} {
            error [list No Such File]
        }

        if {![file readable $name]} {
            error [list File not Readable]
        }
        set format {1024/2 "0x%04x " "\n"}
        set fd [open $name r]
        fconfigure $fd -encoding binary -translation binary

    }
    # close:
    #      Close the current event sourc if it is open.
    # Implicit Inputs:
    #     fd  - File descriptor open on the event source.
    method close {} {
        if {$fd != [list]} {
            catch {close $fd}
            set fd [list]
        }
    }
    # getNext
    #   Reads the next buffer from the data source.
    # Implicit inputs:
    #    fd     - The file descriptor open on the file.
    #    partial- Any partial line remaining after the prior
    #             buffer filled up.
    #
    # Errors:
    #   {End of Data Source}  - End file was read on the data file.
    #
    method getNext {} {
        if {[eof $fd]} {
            error [list End of Data Source]
        }
        set binaryData [read $fd $options(-buffersize)]
        set size   [expr {$options(-buffersize)/2}]
        binary scan $binaryData s$size buffer
        return $buffer

    }
}
#-----------------------------------------------------------------
#  onlineDataSource
#       This class allows you to connect to an NSCL/Spectrodaq
#       online data source.  The requirements for this are
#       -  The local host must have an installation of the
#          spectrodaq client and nscldaq software
#          (spectclonline in particular).
#       -The remote system must be running the spectrodaq server process.
# Options:
#    -daqroot      - Specifies the location of the nscldaq software
#                    defaults to /usr/opt/daq/current
#    -host         - specifies the host from which to accept online data.
#                    defaults to localhost.
#    -buffersize   - Size of the buffer in bytes.
#
#
snit::type onlineDataSource {
    option -daqroot     {/usr/opt/daq/current}
    option -host        localhost
    option -buffersize   8192

    variable havedns     0
    variable fd          {}
    variable partial     {}
    variable buffer      {}
    variable fullBuffer  {}
    variable bufferCount 0

    constructor args {
        $self configurelist $args

        # Setup the nameserver based on /etc/resolv.conf
        # Assumption :  Unix system.
        # Bad thing:    The last nameserver in the /etc/resolv.conf
        #               is used which is sort of the reverse of what's
        #               usually intended.
        #
        if {[file readable /etc/resolv.conf]} {
            set fd [open /etc/resolv.conf r]
            while {![eof $fd]} {
                set line [gets $fd]
                set switch [lindex $line 0]
                set value  [lrange $line 1 end]
                switch -exact $switch  {
                    nameserver {
                        ::dns::configure -nameserver $value
                    }
                    search  {
                        ::dns::configure -search [list $value]
                    }
                }
            }
            set havedns 1
        }
    }
    destructor {
        $self close
    }
    ####
    #   open
    #      Opens the data source.
    # Implicit inputs:
    #   options(-daqroot)   - where to find the daq software.
    #   options(-host)      - host from which data will come.
    # Errors:
    #   {No Local DAQ Installation} Prerequisite software not installed on this
    #                               system.
    #
    method open {} {

        #  Check for the spectcldaq executable.

        set bindir [file join $options(-daqroot) bin]
        set sourceprog [file join $bindir spectcldaq]
        if {![file executable $sourceprog]} {
            error [list No Local DAQ Installation]
        }
        #  If dns has been configured, we can attempt to validate the
        #  host.
        #
        set host $options(-host)
        if {$havedns  && ($host ne "localhost") } {
            set query [::dns::resolve $host]
            set result [::dns::address $query]
            ::dns::cleanup $query
            if {[llength $result] == 0}  {
                error {No Such Host}
            }
        }
        #  There's a remote daq if there's a server listening on
        #  sdaq-link.

        if {[catch {socket $host sdaq-link} msg]} {
            error [list No DAQ on Remote Node]
        } else {
            close $msg
        }
        #  Now we can setup the online client piped through hexdump.
        # ... and a file-event so that we can keep processing buffers.
        # when the event loop is entered.
        #

        set spectcldaq [file join $options(-daqroot) bin spectcldaq]
	set url "tcp://$host:2602/"

        set fd [open [list | $spectcldaq $url]  r]
        fconfigure $fd -buffering line -buffersize $options(-buffersize) \
            -encoding binary -translation binary
        fileevent $fd readable [mymethod gotData]
    }
    #####
    #  close
    #      Close the data source.
    #
    method close {} {
        if {$fd ne [list]} {
            catch {fileevent $fd readable {}}
            catch {close $fd}
            set fd {}
        }

    }
    #####
    #  gotData
    #      Called when data is available from the DAQ
    #      system.  We read until either a buffer is
    #      full or we would block.
    #
    method gotData {} {

        if {[eof $fd]} {
            $self close
            error [list End file on online data source]
        }
        set binaryData [read $fd $options(-buffersize)]
        set size   [expr {$options(-buffersize)/2}]
        binary scan $binaryData s$size fullBuffer
        incr bufferCount

    }
    #####
    # getNext
    #     Get the next buffer from the data source.
    #
    method getNext {} {

        append varname $selfns :: bufferCount
        vwait $varname

        if {[llength $fullBuffer] == 0} {
            error [list End of Data Source]
        }
        return $fullBuffer

    }
}
#---------------------------------------------------------------------
# dataSource
#     This is a container for data sources.
#     the ideas is that you create one of these and use it's open
#     members to create/open the appropriate type of actual data source.
#     Methods like close and getNext are essentially delegated to the
#     data source..which is held as a variable by this object.

snit::type dataSource {
    variable actualSource {}

    constructor args {
    }
    destructor {
        if {$actualSource ne {}} {
            $self close
        }
    }
    ####
    # openOffline file args
    #    creates and opens an offline data source.
    # Parameters:
    #    file   - Name of the file to open as a sourcde.
    #    args   - List of option names/values for
    #             the construction of the offline source.
    #
    method openOffline {file args} {
        set actualSource [eval offlineDataSource  %AUTO% -file $file $args]
        $actualSource open
    }
    #####
    #  openOnline host args
    #    creates and opens an online data source.
    # Parameters:
    #    host  - host from which to get data.
    #    args  -
    method openOnline {host args} {
        set actualSource [eval onlineDataSource %AUTO% -host $host $args]
        $actualSource open
    }
    #####
    # close
    #   Close the actual data source.
    #
    method close {} {
        $actualSource close
        $actualSource destroy
        set actualSource [list]
    }
    #####
    # getNext
    #    Get next buffer from the data source:
    #
    method getNext {} {
        return [$actualSource getNext]
    }
}

