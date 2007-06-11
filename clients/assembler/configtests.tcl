#
#   Test suite for the assembler command configuration.
#
#
package require tcltest
package require dns
package require logger
namespace import ::tcltest::*

# Dns is noisy on the debug level...by default.

::logger::disable debug

# We need to have some nodes that we can count on.  For any given system
# we can really only count on localhost.. we'll use that when we only
# have single node tests.. If we need multiple nodes, however
# we will need to have the user define some local hosts for us below:


set node1 thechad.nscl.msu.edu;                   # May work if dns is good.
set node2 spdaq22.nscl.msu.edu;                   # Should work if dns is good.    
set badIP 123.321.111.222;                        # IP that is known not to exist...
set nameserver trebuchet.nscl.msu.edu


::dns::configure -nameserver $nameserver

################## support functions ########################
proc firstMessageLine msg {
    set lines [split $msg "\n"]
    return [lindex $lines 0]
}

proc createNodeListing {dnsName id window offset} {
    
    # Gotta get the ip address for the node:
    
    set token [::dns::resolve $dnsName]
    ::dns::wait $token
    set addresses [::dns::address $token]
    set names     [::dns::name    $token]
    :::dns::cleanup $token
    
    set ip   [lindex $addresses 0]
    set name [lindex $names     0]
    
    set result [list $name $ip $id $window $offset]
    
    return $result
}


################ tests on empty databases #####################

test emptylist-1.0  {assembler list for an empty database}  \
-setup {assembler clear}                                    \
-body  {
        set result [assembler list]
}                                                           \
-result {{} {}}

test emptylist-1.1 {assembler validate for an emtpy database} \
-setup {assembler clear}   \
-body {
    catch {assembler validate} msg
    set msg
}                                                              \
-result {No trigger node specified}


test emptylist-1.2 {Attempt to set a trigger on an empty database}  \
-setup {assembler clear}                                            \
-body {
    catch {assembler trigger 1234} msg
    #
    #  Only care about the first line of the msg:
    
    set result [firstMessageLine $msg]
    
    
}                                                                   \
-result {There is no node with this node id}

test emptylist-1.3 {Attempt to set a window on an empty database} \
-setup {assembler clear}                                          \
-body {
    catch {assembler window 1234 15} msg
    set result [firstMessageLine $msg]
}                                                                 \
-result {There is no node with this node id}

##############################Tests with node definitions: #########
#
  

test node-1.0   {Define a good node and list it}             \
-setup {assembler clear}                                \
-body {
    assembler node $node1 1234
    set listing [assembler list]
    
    # Construct what this should look like:
    
    set shouldbe   [list ""];              # Trigger.
    lappend nodes  [createNodeListing $node1 1234 "" ""]
    lappend shouldbe $nodes
    
    if {$listing eq $shouldbe} {
        set result 1
    } else {
        set result 0
    }
} \
-result {1}

test node-1.1 {Define a node with a bad hostname}               \
-setup {assembler clear}                                        \
-body {
    catch {assembler node this.name.cant.exist 1234} msg
    set result [firstMessageLine $msg]
}                                                               \
-result {This node cannot be found in DNS}

test node-1.2 {Define a node with a bad IP address}             \
-setup {assembler clear}                                        \
-body {
    catch {assembler node $badIp} msgcat::msg
    set result [firstMessageLine $msg]
    
}                                                               \
-result {This node cannot be found in DNS}

test node-1.3 {Define a node with a non numeric id}         \
-setup {assembler clear}                                    \
-body {
    catch {assembler node $node1 abcde} message
    set result [firstMessageLine $message]
    
}                                                           \
-result {Invalid node id, node ids are positive integers less than 65536}

test node-1.4 {Define a node with a negative id}            \
-setup {assembler clear}                                    \
-body {
    catch {assembler node $node1 -5} message
    set result [firstMessageLine $message]
}                                                           \
-result {Invalid node id, node ids are positive integers less than 65536}

test node-1.5 {Define a node with a huge id}                \
-setup {assembler clear}                                    \
-body {
    catch {assembler node $node1 123456} message
    set result [firstMessageLine $message]
}                                                           \
-result {Invalid node id, node ids are positive integers less than 65536}

########################### tests of the trigger command:

test trigger-1.0 {Define a node and define it as the trigger} \
-setup {assembler clear}                                    \
-body {
    assembler node $node1 80
    assembler trigger 80
    set listing [assembler list]
    set result [lindex $listing 0]
}                                                           \
-result {80}

test trigger-1.1 {Attempt to define a nonexisting node as the trigger} \
-setup {assembler clear}                                                \
-body {
    catch {assembler trigger 80} errorMessage
    set result [firstMessageLine $errorMessage]
}                                                                       \
-result {There is no node with this node id}

test trigger-1.2 {Define trigger with a negative node number} \
-setup {assembler clear}                                    \
-body {
    catch {assembler trigger -1} errorMessage
    set result [firstMessageLine $errorMessage]
}                                                           \
-result {Invalid node id, node ids are positive integers less than 65536}


test trigger-1.3 {Define trigger with too large a node number}      \
-setup {assembler clear}                                            \
-body {
    catch {assembler trigger 70000} errorMessage
    set result [firstMessageLine $errorMessage]
}                                                                      \
-result {Invalid node id, node ids are positive integers less than 65536}

test trigger-1.4 {Ensure we can override and existing trigger}  \
-setup {assembler clear}                                        \
-body {
    assembler node $node1 80
    assembler node $node2 81
    
    assembler trigger 80
    set trigger1 [lindex [assembler list] 0]
    assembler trigger 81
    set trigger2 [lindex [assembler list] 0]
    
    set result [list $trigger1 $trigger2]
    
}                                                               \
-result {80 81}


#################### Test the assembler window command #################

test window-1.0 {Set a valid window on a node consisting only of a width} \
-setup {
    assembler clear
    assembler node $node1 80
}                                                                          \
-body {
    assembler window 80 250
    set info [assembler list]
    set nodelist [lindex $info 1]
    set nodeinfo [lindex $nodelist 0]
    set width    [lindex $nodeinfo 3]
    set offset   [lindex $nodeinfo 4]
    
    set result [list $width $offset]
    
}                                                                       \
-result {250 0}

test window-1.1 {Set window with a width and an offset}     \
-setup {
    assembler clear
    assembler node $node1 80
}                                                                          \
-body {
    assembler window 80 250 50
   set info [assembler list]
    set nodelist [lindex $info 1]
    set nodeinfo [lindex $nodelist 0]
    set width    [lindex $nodeinfo 3]
    set offset   [lindex $nodeinfo 4]
    
    set result [list $width $offset]
}                                                                       \
-result {250 50}

test window-1.2 {Window widths must be positive}                \
-setup {
    assembler clear
    assembler node $node1 80
}                                                                          \
-body {
    catch {assembler window 80 -10} errorMessage
    set result [firstMessageLine $errorMessage]
}                                                       \
-result {The window width must be a positive integer}

test window-1.3 {Window  widths must be valid integers}             \
-setup {
    assembler clear
    assembler node $node1 80
}                                                                          \
-body {
    catch {assembler window 80 justplainwrong} errorMessage
    set result [firstMessageLine $errorMessage]
}                                                       \
-result {The window width must be a positive integer}

test window-1.3 {offsets must be valid integers}            \
-setup {
    assembler clear
    assembler node $node1 80
}                                                                          \
-body {
    catch {assembler window 80 123 justPlainWrong} errorMessage
    set result [firstMessageLine $errorMessage]
}                                                           \
-result {The offset must be a valid integer}

########################## Test the validate command ########################

test validate-1.0  {Test good validation}               \
-setup {
    assembler clear
    assembler node $node1 80
    assembler node $node2 81
}                                                       \
-body {
    assembler window 80 100
    assembler window 81 110
    assembler trigger 81
    set result [assembler validate]
    
}                                           \
-result {}

test validate-1.1 {Test validation but with no trigger set} \
-setup {
    assembler clear
    assembler node $node1 80
    assembler node $node2 81
}                                                       \
-body {
    assembler window 80 100
    assembler window 81 110
    catch {assembler validate} errorMessage
    set result errorMessage
}                                               \
-result {No trigger node has been defined}

test validate-1.0 {Test validation, but a node has no window}   \
-setup {
    assembler clear
    assembler node $node1 80
    assembler node $node2 81
}                                                               \
-body {
    assembler window 80 100
    assembler trigger 81
    catch {assembler validate} errorMessage
    
    # construct the error message:
    
    
    set expected "Node $node2 - id 81  has not had a matching width specified"
    
    set result [string compare $expected $errorMessage]
    
}                                                               \
-result 0



################################### Run the tests and exit  #######
cleanupTests

exit
