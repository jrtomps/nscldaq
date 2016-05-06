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
# @file   makeTest.py
# @brief  Make a test event builder and data source.
# @author <fox@nscl.msu.edu>

#  We must have created a data base file in the home directory named
#  test.db that has already got the run state stuff built
#

from nscldaq.vardb import VardbEvb
import getpass
import os

uri = 'file://' + os.getenv('HOME') + '/test.db'
api = VardbEvb.VardbEvb(uri)

api.createSchema()

#  Remove the event builder 'test' in case it already exists:

try:
    api.rmEventBuilder('test')
except:
    pass

api.createEventBuilder('test', 'charlie', 100, 'hehehe', serviceSuffix='hehehe')
api.addDataSource(
    'test', 'fox', 'charlie', os.getenv('DAQBIN') + '/ringFragmentSource',
    'tcp://charlie/fox', [1,2],
    info='Test data source'
)


