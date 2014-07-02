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
# @file   NotReady.py
# @brief  Implements the NotReady state of the state manager.
# @author <fox@nscl.msu.edu>

##
# stub:

import time

##
# NotReady
#   Implements the not ready state.
#   -------------- STUB ---------------------------
#
# @param cargo - data passed from the state machine.
#
def NotReady(cargo):
    print("NotReady entered")
    while True:
        time.sleep(1)
    #
    #   Never leave this state.
    #

