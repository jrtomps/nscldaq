#!/usr/bin/env python



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
# @file   stateManager.py
# @brief  Main for the state manager application.
# @author <fox@nscl.msu.edu>

##
# The state manager application supplies a central server that manages
# the state of an experiment.

from nscldaq.statemanager import StateMachine, NotReady


stateMachine = StateMachine.StateMachine()
stateMachine.add_state('NotReady', NotReady.NotReady)
stateMachine.set_start('NotReady')

## Dummy end state:
#

stateMachine.add_state('Exit', None, 1)

##
# Run the state machine:

stateMachine.run(None)



