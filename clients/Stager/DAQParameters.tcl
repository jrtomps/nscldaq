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
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

# This package captures and encapsulates the
# configuration information of the data acquisition subsystem.
# This configuration data controls how Readout is started, what
# it is fed for a first run number, and how data are recorded.
# Configuration parameters are:
#
# Parameter Name   Env Name      Meaning
# SourceHost       DAQHOST       Which system runs the readout software
# ReadoutPath      RDOFILE       Full path to the readout program.
# FtpHost          EVTHOST       Host on which event recording is done.
# Password         PASSWD        An encrypted password.  The password is
#                                encrypted via DES with a key you'll just
#                                have to dig out yourself.  Note that
#                                If supplied in the env var, PASSWD is assumed
#                                to be cleartext and encrypted here.
# BufferSize       BUFFERSIZE    Size of buffers on disk.
#

package provide DAQParameters 1.0
package require Configuration
package require des


namespace eval DAQParameters {
}
# DAQParameters::setDefaults
#  Initialize the default values for each of the parameters.
#  The following parameters have defaults:
#
#    SourceHost   - localhost
#    BufferSize   - 4096 (words)
proc DAQParameters::setDefaults {} {
    
    Configuration::Set SourceHost localhost
    Configuration::Set BufferSize  8192
}
# DAQParameters::environmentOverrides
#   Overrides the defaults with information from the environment
#   See the file comment header for information about which
#   items support env overrides.
#
proc DAQParameters::environmentOverrides {} {
    global env


    Configuration::readEnvironment SourceHost  DAQHOST localhost
    Configuration::readEnvironment ReadoutPath RDOFILE
    Configuration::readEnvironment FtpHost     EVTHOST
    Configuration::readEnvironment BufferSize  BUFFERSIZE 8192


    #  Passwd is handled strangely since it must be encrypted.

    if {[array names env PASSWD] != ""} {
        DAQParameters::passwordIs $env(PASSWD)
    }
}
# DAQParameters::getSourceHost
#    Returns the name of the host on which Readout should be run
#    (DAQHOST).
#
proc DAQParameters::getSourceHost {} {
    return [Configuration::get SourceHost localhost]
}
# DAQParameters::sourceHostIs name
#    Set a new value for the data source host.
# Parameters:
#    name    - Name of the host.
#
proc DAQParameters::sourceHostIs {name} {
    Configuration::Set SourceHost $name
}
# DAQParameters::getReadoutPath
#     Returns the current path to the readout program.
#
proc DAQParameters::getReadoutPath {} {
    return [Configuration::get ReadoutPath]
}
# DAQParameters::readoutPathIs path
#    Sets a new readout path.
#    The path must point to an executable file.
#
# Parameters:
#   path   - Path to the file.
# Errors:
#    NotFound       - File is not found.
#    NotExecutable  -  File is not executable.
proc DAQParameters::readoutPathIs path {

    if {![file exists $path]} {
        error DAQParameters::NotFound
    }
    if {![file executable $path]} {
        error DAQParameters::NotExecutable
    }
    Configuration::Set ReadoutPath $path
}
# DAQParameters::getFtpHost
#    Returns the name of the host to which ftp
#    event logging is directed.
#
proc DAQParameters::getFtpHost {} {
    return [Configuration::get FtpHost]
}
# DAQParameters::ftpHostIs host
#   Sets the name of the ftp host.  The host must exist.
#   and be running an ftp server.  This is checked for
#   via attempting to form a connection to it on the ftp
#   port.
# Parameters:
#   host    - the ftp host. Can be either a DNS name or
#             dotted ip.
# Errors
#    HostNotFTPServer
#
proc DAQParameters::ftpHostIs {host} {
    if {[catch [list socket $host ftp] sock] == 0} {
        close $sock
        Configuration::Set FtpHost $host
    } else {
        error DAQParameters::HostNotFTPServer
    }
}
# DAQParameters::passwordIs passwd
#    Sets a new password.  As this is assumed to come from the
#    outside world, we get a plaintext password. This is stored
#    des encrypted so that when written to the config file it
#    can't be easily read.
# Parameters:
#   passwd   - new password.
#
proc DAQParameters::passwordIs {passwd  {key {}}} {
    global tcl_platform

    set o $tcl_platform(user)
    set e nscldaq
    set f {ron fox}




    set enc [::DES::des -mode encode -key $key$o$key$e$key$f$key  $passwd]
    Configuration::Set Password $enc
}
# DAQParameters::getPassword key
#   Returns the unencrypted password given that you know the encryption key.
#
proc DAQParameters::getPassword {{key {}}} {
    global tcl_platform

    set o {ron fox}
    set e $tcl_platform(user)
    set f nscldaq

    set cipher [Configuration::get Password]
    set word [::DES::des -mode decode -key $key$e$key$f$key$o$key $cipher]
    set word [string trim $word "\0"]


    return $word
}
# DAQParameters::setEncryptedPasswd ciphertext
#    Set the ciphertext Password
# Parameters:
#    ciphertext  - encrypted passwd.
proc DAQParameters::setEncryptedPasswd ciphertext {
    Configuration::Set Password $ciphertext
}
# DAQParameters::getEncryptedPasswd
#    Returns encrypted passwd.
#
proc DAQParameters::getEncryptedPasswd {} {
    return [Configuration::get Password]
}

# DAQParameters::setBufferSize value
#    Set a buffersize for daq (used to name event files).
# Parameters:
#   value  - New buffersize.
# Errors:
#   InvalidSize  - value < 128.
proc DAQParameters::setBufferSize value {
    if {$value < 128} {
        error DAQParameters::InvalidSize
    }
    Configuration::Set BufferSize $value
}
# DAQParameters::getBuffersize
#    Returns the current buffersize.
#
proc DAQParameters::getBufferSize {} {
    return [Configuration::get BufferSize 8192]
}

