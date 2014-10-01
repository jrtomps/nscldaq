

package provide MockCCUSB 1.0

package require snit


namespace eval MockCCUSB {

  ## @brief a Mock of the CCUSB API to record command histories
  #
  snit::type Controller {

    variable m_history  ;#< the command history

    ## @brief Constructor
    #
    # @param args   option-value pairs
    #
    # @returns  fully-qualified name of new instance
    constructor {args} {
      set m_history [list]
    }

    # -------------------------------------------------------------------------
    #  Single shot operations
    #
    #  These all just append the name of the command and arguments to the
    #  history


    method simpleWrite24 {n a f data} {
      lappend m_history "simpleWrite24 $n $a $f $data"
    }

    method simpleRead24 {n a f} {
      lappend m_history "simpleRead24 $n $a $f"
    }

    method simpleControl {n a f} {
      lappend m_history "simpleControl $n $a $f"
    }

    ## @brief Append all entries of the stack to the history
    #
    # @param stack  a MockCCUSB::ReadoutList
    # @param max    not used
    method executeList {stack max} {
      set stackHistory [$stack getHistory]
      foreach cmd $stackHistory {
        lappend m_history $cmd
      }
    }

    ## @brief Retrieves the history of commands
    #
    method getHistory {} {
      return $m_history
    }

  } ;# end Controller


  ## @brief A fake CCCUSBReadout list that maintains creates a cmd history
  #
  snit::type ReadoutList {
    variable m_history  ;#< the command history

    ## @brief Constructor
    #
    # @param args   option-value pairs
    constructor {args} {
      set m_history [list]
    }

    ## @brief Stack building commands
    # 
    # These all simply add to the history a description of the command
    #

    method addControl {n a f} {
      lappend m_history "control $n $a $f"
    }

    method addRead16 {n a f {lamWait 0}} {
      lappend m_history "read16 $n $a $f $lamWait"
    }

    method addRead24 {n a f {lamWait 0}} {
      lappend m_history "read24 $n $a $f $lamWait"
    }

    method addWrite16 {n a f data} {
      lappend m_history "write16 $n $a $f $data"
    }

    method addWrite24 {n a f data} {
      lappend m_history "write24 $n $a $f $data"
    }


    method addAddressPatternRead16 {n a f {lamWait 0}} {
      lappend m_history "addressPatternRead16 $n $a $f $lamWait"
    }

    ## @brief Retrieves the history
    #
    # @returns a list of the commands called
    #
    method getHistory {} {
      return $m_history
    }

  } ;# end ReadoutList


} ;# end namespace

