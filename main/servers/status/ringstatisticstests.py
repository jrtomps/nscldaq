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
# @file   ringstatisticstests.py
# @brief  Tests of python bindings to CRingStatisticsMessage
# @author <fox@nscl.msu.edu>

import unittest
from nscldaq.status import statusmessages
import zmq

class TestRingStatistics(unittest.TestCase):
    def setUp(self):
        statusmessages.enableTest()
    def tearDown(self):
        statusmessages.disableTest()
        
    ############################  the tests ####################################
    
    def test_construct(self):
        pass
    
if __name__ == '__main__':
    unittest.main()


