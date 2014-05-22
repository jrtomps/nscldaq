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
# @file   projectLoggerTest.py
# @brief  Test methods of project.Loggers
# @author <fox@nscl.msu.edu>

import unittest
import project

class projectLoggerTest(unittest.TestCase):
    def setUp(self):
        self._project = project.Project(":memory:")
        self._project.create()
        self._loggers = project.Loggers(self._project)
        self._rings   = project.Rings(self._project)
        self._hosts   = project.Hosts(self._project)
        self._programs= project.Programs(self._project)
        
        # Create some ring, hosts, and programs.
        
        # Make some hosts, rings and programs...maintain information about
        # them in attributes so that they can be used in the tests
        self.hids = list()
        self.pids = list()
        self.rids = list()
        
        self.hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        self.rings= ('test', 'fox', 'fox1')
        self.sourcids=(1,2,3)
        self.paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        self.wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        self.args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd,a,r,s in zip(self.hosts, self.paths, self.wds, self.args, self.rings, self.sourcids):
            hid = self._hosts.add(h)
            self.hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            self.pids.append(pid)
            self.rids.append(self._rings.add_withHostId(r, hid, s))

        
    #--------------------------------------------------------------------------
    # Tests for the add method.
    
    ##
    #  test_add_ok

    def test_add_ok(self):
        self._loggers.add(self.rids[1], self.hids[1], self.wds[1])
        
        curs = self._project.connection.cursor()
        curs.execute('''
            SELECT * FROM loggers
        ''')
        row = curs.fetchone()
        self.assertNotEquals(None, row)
        
        self.assertEquals(1, row[0])
        self.assertEquals(self.rids[1], row[1])
        self.assertEquals(self.hids[1], row[2])
        self.assertEquals(None, row[3])
        self.assertEquals(self.wds[1], row[4])
        
        row = curs.fetchone()
        self.assertEquals(None, row)
        

    ##
    # test_add_mylogger_ok
    
    def test_add_mylogger_ok(self):
        self._loggers.add(self.rids[1], self.hids[1], self.wds[1],self.pids[2])
        curs = self._project.connection.cursor()
        curs.execute('''
            SELECT * FROM loggers
        ''')
        row = curs.fetchone()
        self.assertNotEquals(None, row)
        
        self.assertEquals(1, row[0])
        self.assertEquals(self.rids[1], row[1])
        self.assertEquals(self.hids[1], row[2])
        self.assertEquals(self.pids[2], row[3])
        self.assertEquals(self.wds[1], row[4])
  
         
        row = curs.fetchone()
        self.assertEquals(None, row)
        
    ##
    # test_add_badring
    #    Bad ring id is a RuntimeError:
    
    def test_add_badring(self):
        thrown = False
        try:
           self._loggers.add(1234, self.hids[1], self.wds[1],self.pids[2])
        except RuntimeError:
            thrown = True
        except:
            pass    
        self.assertTrue(thrown)
    ##
    # test_add_badhost
    #  Bad host id is a runtime error too:
    
    def test_add_badhost(self):
        thrown = False
        try:
           self._loggers.add(self.rids[1], 4321, self.wds[1], None)
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)
        
    ##
    # test_add_badprogid
    
    def test_add_badprogid(self):
        thrown = False
        try:
            self._loggers.add(self.rids[1], self.hids[1], self.wds[1], 1234)
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)


    #---------------------------------------------------------------------------
    #  Tests for add_specified.
    #

    ##
    # test_addspec_ok1
    #
    #   Specified by ring id, loggerhostid. no program.
    def test_addspec_ok1(self):
        path = "/scratch/fox/events"
        self._loggers.add_Specified(
            ringId=self.rids[1], loggerHostId=self.hids[2], logPath=path)

        curs = self._project.connection.cursor()       
        curs.execute('''SELECT ring_id, host_id, log_dir FROM loggers''')
        
        r  = curs.fetchone()
        self.assertNotEquals(None, r)
        self.assertEquals(self.rids[1], r[0])
        self.assertEquals(self.hids[2], r[1])
        self.assertEquals(path, r[2])
        
    ##
    # test_addspec_ok2
    #
    #   Specified by ring name/ringhost loggerhostid, no program.
    
    def test_addspec_ok2(self):
        path="/scratch/fox/events"
        self._loggers.add_Specified(
            ringName=self.rings[1], ringHost=self.hosts[1],
            loggerHostId=self.hids[2], logPath=path
        )
        
        curs = self._project.connection.cursor()
        curs.execute('''SELECT ring_id, host_id, log_dir FROM loggers''')
        
        r = curs.fetchone()
        self.assertNotEquals(None, r)
        self.assertEquals(self.rids[1], r[0])
        self.assertEquals(self.hids[2], r[1])
        self.assertEquals(path, r[2])        
    
    ##
    # test_addspec_ok3
    #
    #    Specified by ring id, logger hostname no program.
    
    def test_addspec_ok3(self):
        path = "/scratch/fox/events"
        self._loggers.add_Specified(
            ringId=self.rids[1], loggerHostName=self.hosts[2], logPath = path
        )

        curs = self._project.connection.cursor()
        curs.execute('''SELECT ring_id, host_id, log_dir FROM loggers''')
        
        r = curs.fetchone()
        self.assertNotEquals(None, r)
        self.assertEquals(self.rids[1], r[0])
        self.assertEquals(self.hids[2], r[1])
        self.assertEquals(path, r[2])        
    
    ##
    #  test_addspec_ringoverspecified
    #    Ringid is there but so is ringname, ring host.
    
    def test_addspec_ringoverspecified(self):
        path="/scratch/fox/events"
        threw = False
        
        try:
            self._loggers.add_Specified(
                ringId=self.rids[1], ringName=self.rings[0],
                ringHost=self.hosts[0]
            )
        except RuntimeError:
            threw = True
        except:
            pass
        
        self.assertTrue(threw)
    
    ##
    # test_addspec_ringunderspecified1
    #    Only ring name not host
    
    ##
    # test_addspec_ringunderspecified2
    #   Only ring id not host
    
    ##
    # test_addspec_noring
    #    None of the ring specifiers are present.
    
    ##
    # test_addspec_badringname
    #   Ring name/host speified but does not translate to a ring id.
    
    ##
    # test_addspec_hostoverspecified
    #   Both loggerhostid and loggerhostname specified.
    
    ##
    # test_addspec_hostunderspecified
    #   none of loggerhostid/logger hostname specified.
    
    
    ##
    # test_addspec_nohost
    #    loggerhostname specified but not a host.


    ##
    # test_addspec_nologpath
    #   logpath not specified,.
    
    ##
    # test_addspec_noprogram
    #    Program id was specified but no good.
    
    ##
    # test_addspec_badattribute
    #   Specification has a bad attribute.






if __name__ == '__main__':
    unittest.main()