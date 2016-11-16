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
# @file   PortManagerTests.py
# @brief  Tests of the port manager API for python.
# @author <fox@nscl.msu.edu>


import unittest
from nscldaq.portmanager import PortManager
import errno
import socket
import getpass


##
# @class PortManagerTests
#
#   Unit tests for the port manager.
#
class PortManagerTests(unittest.TestCase):
    def setUp(self):
        pass
    def tearDown(self):
        pass
    
    #
    # turn port info list into a dict keyed by the service.
    #
    def _keyInfo(self, info):
        keyedData = {}
        for aport in info:
            service = aport['service']
            keyedData[service] = aport
        return keyedData
    
    ##
    # test_connection_fail
    #   Connecting to a nonexistent port manager throws gaierror
    #   with -5 (?) error.
    #
    def test_connection_fail(self):
        host='no.such.host.nscl.msu.edu'
        port=30000
        threw      =  False
        rightThrow = False
        rightErrno = False
        try:
            pm = PortManager.PortManager(host, port)
        except socket.gaierror as e:
            threw = True
            rightThrow = True
            if e.errno in (-2, -5):
                rightErrno = True
            else:
                print("Errno gotten was ", e.errno)
        except:
            threw = True
            
        self.assertTrue(threw)
        self.assertTrue(rightThrow)
        self.assertTrue(rightErrno)
        
    ##
    # test_connection_ok
    #   This time we should be able to form a connection:
    #
    def test_connection_ok(self):
        host='localhost'
        port=30000
        threw = False
        
        try:
            pm = PortManager.PortManager(host, port)
        except:
            threw = True
        self.assertFalse(threw)
        
    
    ##
    # test_getPort_ok
    #
    #   We should be able to get a port from the server:
    #
    def test_getPort_ok(self):
        pm = PortManager.PortManager('localhost', 30000)
        port = pm.getPort('myport')
        self.assertTrue(isinstance(port, (int, long)))
    
    
        
    ##
    # test_list_base
    #   Tests that the port manager can give us the list initially.  The
    #   assumption is that nothing else is running in which case
    #   for nscldaq-12 there will be the following services:
    #   - RingMaster
    #   - StatusAggregatorPull
    #   - StatusAggregatorPUB
    #
    #   We don't care about the
    #   port or the user.
    #
    def test_list_base(self):
        pm    = PortManager.PortManager('localhost', 30000)
        ports = pm.listPorts()
        self.assertEquals(3, len(ports))
        rm = ports[0]
        self.assertEquals('RingMaster12', rm['service'])
    
    ##
    # test_list_afew
    #   Not content with ensuring this all works for a single port I'll create
    #   a few and ensure the all are listed.
    #   - This test assumes the standard services are running.
    #
    def test_list_afew(self):
        pm     = PortManager.PortManager('localhost', 30000)
        myport = pm.getPort('myport')
        theport= pm.getPort('theport')
        iam    = getpass.getuser()
        
        info   = pm.listPorts()
        self.assertEquals(5, len(info))
        
        # Toss the data up into a dict keyed by service name.
        
        keyedData = self._keyInfo(info)
    
        # RingMaster is present:
        
        self.assertTrue('RingMaster12' in keyedData.keys())
        
        # myport is present and has the right stuff:
        
        self.assertTrue('myport' in keyedData.keys())
        self.assertEquals(myport, keyedData['myport']['port'])
        self.assertEquals(iam,    keyedData['myport']['user'])
        
        # theport is present and has the right stuff:
        
        self.assertTrue('theport' in keyedData.keys())
        self.assertEquals(theport, keyedData['theport']['port'])
        self.assertEquals(iam,     keyedData['theport']['user'])

    ##
    # test_find_byservice
    #   Test find given a service to  look for (exact match).
    #
    def test_find_byservice(self):
        pm     = PortManager.PortManager('localhost', 30000)
        myport = pm.getPort('myport')
        theport= pm.getPort('theport')
        iam    = getpass.getuser()
        
        info = pm.find(service = 'theport')
        self.assertEquals(1, len(info))
        pinfo = info[0]
        
        self.assertEquals('theport', pinfo['service'])
        self.assertEquals(theport,  pinfo['port'])
        self.assertEquals(iam,      pinfo['user'])
    
    
    ##
    # test_find_bybeginswith
    #   Test find given the starting value of a service name:
    #
    def test_find_bybeginswith(self):
        pm     = PortManager.PortManager('localhost', 30000)
        myport = pm.getPort('myport')
        theport= pm.getPort('theport')
        iam    = getpass.getuser()
        
        info   = pm.find(beginswith = 'my')
        self.assertEquals(1, len(info))
        pinfo = info[0]
        
        self.assertEquals('myport', pinfo['service'])
        self.assertEquals(myport,  pinfo['port'])
        self.assertEquals(iam,      pinfo['user'])
    
    ##
    # test_find_byuser
    #   Locate ports with a specific username.
    #   Note that in jenkins we'll get too many ports so we only care about
    #   having the ports we allocated and don't care if we match too many.
    #
    def test_find_byuser(self):
        pm     = PortManager.PortManager('localhost', 30000)
        myport = pm.getPort('myport')
        theport= pm.getPort('theport')
        iam    = getpass.getuser()
        
        info = pm.find(user=iam)
        info = self._keyInfo(info)
        
        # We can assume that if the keys are there, the data are good.
        
        self.assertTrue('myport' in info.keys())
        self.assertTrue('theport' in info.keys())
        
    
    ##
    # test_find_byillegal
    #  Illegal search spec throws RuntimError:
    #
    def test_find_byillegal(self):
        throws   = False
        rthrows  = False
        
        try:
            pm = PortManager.PortManager('localhost', 30000)
            pm.find(george = 'harry')
        except RuntimeError:
            throws = True
            rthrows = True
        
        self.assertTrue(throws)
        self.assertTrue(rthrows)
    
#
# Run the tests.
#
if __name__ == '__main__':
    unittest.main()

