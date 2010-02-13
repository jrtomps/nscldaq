#
#   Definitions of the ring event format.
#
#

#  Known item types:

# State changes:

set BEGIN_RUN   1
set END_RUN     2
set PAUSE_RUN   3
set RESUME_RUN  4

set ControlItems [list $BEGIN_RUN $END_RUN $PAUSE_RUN $RESUME_RUN]

# Documentation buffers (string lists)

set PACKET_TYPE         10
set MONITORED_VARIABLES 11

set StringListItems [list $PACKET_TYPE $MONITORED_VARIABLES]


# other types:

set PHYSICS_EVENT       30
set PHYSICS_EVENT_COUNT 31
