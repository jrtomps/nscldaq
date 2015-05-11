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
# @file   varmgrcommon.py
# @brief  Common bits of python code for the variable database manager.
# @author <fox@nscl.msu.edu>

import os
import os.path
import nscldaq.vardb.varmgr
import sys

# programParent
#   Return the parent directory of the run control programs:
#
# @param db  - database api object.
# @return string - parent directory of the run control programs.
#
def programParent(db):
    parentDir = db.get('/RunState/ReadoutParentDir')
    if parentDir == '':
        parentDir = '/RunState'
    return parentDir


# programDir
#   Change directory to the named program's state directory.
# @param db - The API to cd.
# @param name - Name of the program.
#
def programDir(db, name):
    parentDir = programParent()
    return os.path.join(parentDir, name)
    
# goodPath
#   Determines if a path exists and is executable
#
# @param path - Path to check.
# @return bool - true if the path is good.
def goodPath(path):
    return os.access(path, os.X_OK)

# haveRunState
#   Is there a /RunState directory?
# @param db - database api object.
# @return bool true - yes, false no.
#
def haveRunState(db):
    dirs = db.ls('/')
    
    return 'RunState' in dirs