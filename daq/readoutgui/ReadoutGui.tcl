#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file ReadoutGui.tcl
# @brief Ties the new readout GUI together into a neat bundle.
# @author Ron Fox <fox@nscl.msu.edu>

package provide ReadoutGui 1.0
package require Tk
package require snit
package require ui
package require RunstateMachine
package require eventLogBundle
package require DataSourceManager
package require DataSourceUI
package require rdoCalloutsBundle;     # Auto registers.

##
# @class ProviderList
#
#  Produces a multi-column list of data source providers that are currently
#  running in a modal dialog.  The list is displayed in a ttk::treeview
#  that is used as a table with the following columns:
#  *   source id   - Id of the data source.
#  *   source type - provider name.
#  *   parameter   - Name of a parameter in the source parameterization.
#  *   value       - Value of the parameterization.
#
#  information about a source will take several lines, the first line has the
#  sourceid and type,  the remaining lines the parameterization e.g.:
#
# +-------------------------------------------------------------------+
# | source id | source type | parameter   |  value                    |
# |         1 | SSHPipe     |             |                           |
# |           |             | host        | localhost                 |
# |           |             | path        | /user/fox/readout/Readout |
# |           |             | parameters  |                           |
#  ...
#
# OPTIONS
#   -sources - the list of source dicts from the data source manager.
#
snit::widgetadaptor ProviderList {
    option -sources -default [list] -configuremethod _updateDisplay
    component table
    component dialog
    
    delegate method * to dialog
    
    variable columns [list {Source Id} {Source Type} Parameter Value] 
    ##
    #  constructor
    #     Construct and layout the dialog
    # @param args - configuration parameters.
    #
    constructor args {
        installhull using toplevel
        
        install dialog using DialogWrapper $win.dialog
        set tableContainer [$dialog controlarea]
        set f [ttk::frame $tableContainer.f]
        install table using ttk::treeview $f.table  \
            -xscrollcommand [list $f.hsb set] -yscrollcommand [list $f.vsb set] \
            -columns $columns -display $columns -show headings
        foreach col $columns {
            $table heading $col -text $col
        }
        
        ttk::scrollbar $f.vsb -command [list $table yview] -orient vertical
        ttk::scrollbar $f.hsb -command [list $table xview] -orient horizontal
        
        grid $table $f.vsb -sticky nsew
        grid $f.hsb -sticky nsew
        grid $f  -sticky nsew
        $dialog configure -form $f -showcancel 0
        grid $dialog -sticky nsew
        
        $self configurelist $args
        
        $self modal
    }
    #-------------------------------------------------------------------------
    # Configuration handling
    #
    
    ##
    # _updateDisplay
    #   Updates the contents of the table with current values.
    # @param optname - Name of the option that holds the configuration.
    # @param value   - new value.
    #
    method _updateDisplay {optname value} {
        set options($optname) $value
        
        $table delete [$table children {}]

        foreach source $value {
            set id   [dict get $source sourceid]
            set type [dict get $source provider]
            dict unset source sourceid
            dict unset source provider
            
            # This leaves only the parameterization in source
            
            $table insert {} end -values [list $id $type]
            dict for {param value} $source {
                $table insert {} end -values [list "" "" $param $value] 
            }
            
        }
    }
}

##
# @class ReadoutGuiApp
#
#   This snit::type is the application class.  It interacts with the various
#   components to create a harmoniously integrated whole (or so we hope).
#   Really what this chunk of code mostly needs to do is work with the
#   RunstateMachine singleton to ensure that the following pieces and parts
#   of the system are configured and tied in to state transitions:
#   *  Event logger.
#   *  Data Source Manager
#   *  Readout GUI megawidget.
#   *  Event file monitor (Status bar showing event file segments for current run)
#
#  We also make some adjustments to the user interface:
#
#   * Add a Data Source menu.
#   * Add Data Source->Add...
#   * Add Data Source->Delete...
#   * Add Data Source->List...
#
snit::type ReadoutGuiApp {
    component dataSources
    component readoutGui
    component stateMachine
    component dataSourceMenu

    
    ##
    # constructor
    #    Construct the object:
    #    *  Make the ReadoutGUI megawidget
    #    *  Connect the data source manager to the run state machine
    #    *  Connect the eventLogger to the run state machine
    #    *  Make the menus for the data source manager.
    #
    # @param args - not used as we have no options to be concerned with.
    #
    constructor args {
        install stateMachine using RunstateMachineSingleton %AUTO%
        
        install dataSources  using DataSourcemanagerSingleton %AUTO%
        ::DataSourceMgr::register
        
        ::EventLog::register
        install readoutGUI using ReadoutGUI .gui
        pack .gui
        
        $self _createDataSourceMenu
    }
    
    #--------------------------------------------------------------------------
    #
    #  Private methods:
    #
    
    ##
    # _createDataSourceMenu
    #
    #   Create the data source menu for the data source manger and attach it
    #   to the appropriate callbacks.
    #
    method _createDataSourceMenu {} {
        install dataSourceMenu using ::ReadoutGUIPanel::addUserMenu dataSource {Data Source}
        
        $dataSourceMenu add command -label "Add..." -command [mymethod _addProvider]
        $dataSourceMenu add command -label "Delete..." 
        $dataSourceMenu add separator
        $dataSourceMenu add command -label "List" -command [mymethod _listDataProviders]
        
    }
    ##
    # _addProvider
    #   Work with the DataSourceUI package to prompt for a  data source and
    #   its  parameters...when one is selected and parameterized, it is added
    #   to the set of managed data sources
    #
    method _addProvider {} {
        set provider [DataSourceUI::getProvider \
            [DataSourceManager enumerateProviders]]
        #
        # Prompt for the data source parameters only if a data source was selected:
        #
        if {$provider ne ""} {
            catch {$dataSources load $provider};    # May be loaded.
            set requiredParams [$dataSources parameters $provider]
            set parameters [DataSourceUI::getParameters $requiredParams]
            set parameters [convertPromptedValues $parameters]
           
            $dataSources addSource $provider $parameters
            
            # If the collective system cannot pause, we need to turn off'
            # the pause button:
            
            set caps [$dataSources systemCapabilities]
            if {![dict get $caps canPause]} {
                [::RunControlSingleton::getInstance] configure -pausable 0
            }
        }
    }
    ##
    # _listProviders
    #
    #   Pops up a little dialog that shows the data source providers that have
    #   been registered.  Also shows the status of those providers.
    #
    method _listDataProviders {} {
        set sources [$dataSources sources]
        puts $sources
        ProviderList .providers -sources $sources
        destroy .providers
    }
    #--------------------------------------------------------------------------
    #  Procs
    
    proc convertPromptedValues parameters {
        set result [dict create]
        dict for {key value} $parameters {
            set realValue [lindex $value 2]
            dict set result $key $realValue
        }
        return $result
    }
}
