#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file scalerconfig.tcl
# @brief procs that implement the scaler configuration file language.
# @author Ron Fox <fox@nscl.msu.edu>
#
package provide scalerconfig 1.0
package require namemap
package require channel
package require pagedisplay
package require singleModel
package require ratioModel

namespace eval ::scalerconfig {
    variable pageNumber 0
}

nameMap ::scalerconfig::channelMap;   # Mapping channel names -> Channel objs
nameMap ::scalerconfig::pages;        # Page tab names -> widgets.

#------------------------------------------------------------------------------
# private procs
#

##
# _getPage
#   Lookup page by tabname in pages or throw an appropriate error.
#
# @param tab - tab name of the page
# @return page object
#
proc _getPage tab {
    if {[catch {scalerconfig::pages get $tab} page info]} {
        if {[dict get $info -errorcode] eq "NOSUCHKEY"} {
            error "A page with the tab name $tab does not exist"
        } else {
            error "Failed to lookup $tab page: $page"
        }
    }
    return $page    
}
##
# _getChannel
#   Lookup a channel by its name throw an appropriate error on failure.
#
# @param channel - name of the channel
# @return channel object
#
proc _getChannel {channel} {
    if {[catch {scalerconfig::channelMap get $channel} chan info]} {
        if {[dict get $info -errorcode] eq "NOSUCHKEY"} {
            error "There is no channel named $channel"
        } else {
            error "Failed to lookup $channel channel: $chan"
        }
    }
    return $chan    
}
#------------------------------------------------------------------------------
# Procs that implement the config file commands.
#

##
# channel
#   Define a new channel.  This is of the form:
# \verbatim
#   channel ?options? name index?.source?
# \endverbatim
#
#   Where
#   * options can be a combination of 0 or more of -incremental {1|0}, and
#     -width  bitcount
#   * name is a unique scaler name.
#   * index is a scaler index within a data source.
#   * source if present is the data source id the scaler lives in.  If it is not
#     present, the scaler comes from a source that does not have a body header.
#
# What this produces is:
#  *  A channel object named:   channel_index?.source?
#  *  A channelMap entry that maps the channel name to the channel command.
#
proc channel args {
    set name       [lindex $args end-1]
    set descriptor [lindex $args end];     # index?.source
    set options    [lrange $args 0 end-2]
    
    if {[info commands ::channel_$descriptor] eq "::channel_$descriptor"} {
        error "A channel with index $descriptor already has a name"
    }
    
    if {[catch {::scalerconfig::channelMap add $name [Channel channel_$descriptor {*}$options]} msg opts]} {
        catch {channel_$descriptor destroy};            # In case the channel got made.
        if {[dict get $opts -errorcode] eq "DUPKEY"} {
            set command [::scalerconfig::channelMap get $name]
            set channel [lindex [split $command _] end]            
            error "Channel $name is already defined as $channel"
        }
    }
}
##
# page
#   Define a new page  This command is of the form:
#
# \verbatim
#     page tabname page-title
# \endverbatim
#
#
#  @note To make testing easy/possible, the proc getNotebook is assumed
#        to exist in the containing script and returns the widget name of the
#        note book widget.  This is needed because pages are megawidgets that
#        must be children of the notebook.  Futhermore, addPage is also assumed
#        to be defined to add a new page object to the notebook.
#
proc page {tabname title} {
    set notebook    [getNotebook]
    set newWidget   \
        [join [list $notebook page[incr ::scalerconfig::pageNumber]] .]
    
    #  Try to put the widget into the name map to ensure it's unique:
    
    if {[catch {::scalerconfig::pages add $tabname $newWidget} msg erroropts]} {
        if {[dict get $erroropts -errorcode] eq "DUPKEY"} {
            error "A tab named $tabname has already been defined"
        } else {
            error "Addiing $tabname caused an error: $msg"
        }
    }
    #  Actually make the page, and add it to the notebook:
    
    pageDisplay $newWidget -title $title -tab $tabname
    addPage $newWidget $tabname
    
}
##
# display_single
#
#   Add a single display line to a page.  This is done by locating the appropriate
#   page and channel objects, wrappting the channel in a singleModel object and adding
#   that to the page.
#
# @param tab     - The tab name of the page.  Should be a lookup key in the pages
#                  object.
# @param channel - The name of the channel to add.  Should be a lookup key in the
#                  channel map object.
#
proc display_single {tab channel} {
    set page [_getPage $tab]
    set chan [_getChannel $channel]
    
    $page add [singleModel %AUTO% -channel $chan -name $channel]
    
}
##
# display_ratio
#
#  Add a line to a page that is a ratio of scalers.  This is done by locating
#  the page, the numerator and denominator scalers, wrapping a ratioModel
#  around the scalers and then adding that model to the page.
#
# @param tab         - Name of the page tab.
# @param numerator   - Name of the numerator scaler.
# @param denominator - Name of the denominator scaler
#
proc display_ratio {tab numerator denominator} {
    set page [_getPage $tab]
    set num  [_getChannel $numerator]
    set den  [_getChannel $denominator]
    
    $page add [ratioModel %AUTO% \
        -numerator $num -denominator $den -numeratorname $numerator \
        -denominatorname $denominator]
}