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
# @file   projectConsumerTest.py
# @brief  Tests for the projects.Consumer class an its methods
# @author <fox@nscl.msu.edu>

import unittest
import project

class projectConsumerTest(unittest.TestCase):
    def setUp(self):
        self._project = project.Project(":memory:")
        self._project.create()
        
        self._rings   = project.Rings(self._project)
        self._hosts   = project.Hosts(self._project)
        self._programs= project.Programs(self._project)
        self._consumers = project.Consumers(self._project)

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
    
    def tearDown(self):
        pass
    
    
    #--------------------------------------------------------------------------
    # tests for the add method.

    ##
    # test_add_ok
    #   add will make an entry in the consumers table.
    
    def test_add_ok(self):
        self._consumers.add(self.rids[1], self.pids[1])
        
        cur = self._project.connection.cursor()
        cur.execute(
            '''
            SELECT program_id, ring_id FROM consumers WHERE ring_id = ? AND program_id = ?
            ''', (self.rids[1], self.pids[1])
        )
        row = cur.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(self.pids[1], row[0])
        self.assertEquals(self.rids[1], row[1])
        
    
    ##
    # test_add_noring
    #    If the ring id is nonexistent we should get a RuntimeError:
    #
    def test_add_noring(self):
        threw = False
        try:
            self._consumers.add(1234, self.pids[1])
        except RuntimeError:
            threw = True
        except:
            pass
        
        self.assertTrue(threw)
    ##
    # test_add_noprogram
    #   If the program id is nonexistent that should get a RuntimeError too:
    
    def test_add_noprogram(self):
        threw = False
        try:
            self._consumers.add(self.rids[1], 1234)
        except RuntimeError:
            threw = True
        except:
            pass
        
        self.assertTrue(threw)
    
    ##
    # test_add_alreadyconsumer
    #   RunTime error to add a program that is already a consumer:
    
    def test_add_alreadyconsumer(self):
        self._consumers.add(self.rids[1], self.pids[1])
        threw = False
        try:
            self._consumers.add(self.rids[1], self.pids[1])
        except RuntimeError:
            threw = True
        except:
            pass
        
        self.assertTrue(threw)
    
    #-------------------------------------------------------------------------
    #   Tests for add with named ring,.
    
    ##
    # test_addnamed_ok
    #     Add named with a good host and ring work.
    #
    def test_addnamed_ok(self):
        self._consumers.add_withNamedRing(self.rings[0], self.hosts[0], self.pids[0])
        cursor = self._project.connection.cursor()
        cursor.execute(
            '''
            SELECT program_id, ring_id FROM consumers
                WHERE ring_id =?
                AND   program_id = ?
            ''', (self.rids[0], self.pids[0])
        )
        rec = cursor.fetchone()
        self.assertNotEquals(None, rec)
        self.assertEquals(self.pids[0], rec[0])
        self.assertEquals(self.rids[0], rec[1])
    
    ##
    # test_addnamed_nosuchring
    #    If there's no matching ring, that's a RuntimeError
    #
    def test_addnamed_nosuchring(self):
        thrown = False
        try:
            self._consumers.add_withNamedRing('no such ring', 'no.such.host', self.pids[0])
        except RuntimeError:
            thrown = True
        self.assertTrue(thrown)
        
    #--------------------------------------------------------------------------
    #   Tests for list
    #
    
    ##
    # test_list_empty - no data returns an empty list.
    #
    def test_list_empty(self):
        info = self._consumers.list()
        self.assertEquals(0, len(info))
    
    ##
    # test_list_1 - Single row of data.
    #
    def test_list_1(self):
        self._consumers.add(self.rids[1], self.pids[1])
        info = self._consumers.list()
        self.assertEquals(len(info), 1)
        
        consumer = info[0]
        ringInfo    = consumer['ring_info']
        programInfo = consumer['program_info']
        
        self.assertEquals(1, consumer['id'])
        
        self.assertEquals(self.rids[1],     ringInfo['ring_id'])
        self.assertEquals(self.rings[1],    ringInfo['ring_name'])
        self.assertEquals(self.sourcids[1], ringInfo['sourceid'])
        self.assertEquals(self.hids[1],     ringInfo['host_id'])
        self.assertEquals(self.hosts[1],    ringInfo['host_name'])
    
        self.assertEquals(self.pids[1],     programInfo['program_id'])
        self.assertEquals(self.paths[1],    programInfo['path'])
        self.assertEquals(self.wds[1],      programInfo['working_dir'])
        self.assertEquals(self.hids[1],     programInfo['host_id'])
        self.assertEquals(self.hosts[1],    programInfo['host_name'])
        self.assertEquals(self.args[1],     programInfo['args'])
    
    #--------------------------------------------------------------------------0-
    #   Test for consumers method
    
    ##
    #  test_consumers_empty
    #    Empty database gives an empty match.
    
    def test_consumers_empty(self):
        info = self._consumers.consumers(self.rings[2], self.hosts[2])
        self.assertEquals(0, len(info))
    
    ##
    # test_consumers_nomatch
    #    If the database has elements but we don't match one, then still empty
    #
    def test_consumers_nomatch(self):
        self._consumers.add_withNamedRing(self.rings[0], self.hosts[0], self.pids[1])
        
        info = self._consumers.consumers(self.rings[1], self.hosts[0])
        self.assertEquals(0, len(info))
    
    ##
    # test_consumers_1
    #    Matching consumer.
    
    def test_consumers_1(self):
        self._consumers.add_withNamedRing(self.rings[0], self.hosts[0], self.pids[1])
        
        info = self._consumers.consumers(self.rings[0], self.hosts[0])
    
        self.assertEquals(1, len(info))
        prog = info[0]
        self.assertEquals(self.pids[1],  prog['program_id'])
        self.assertEquals(self.paths[1], prog['path'])
        self.assertEquals(self.wds[1],   prog['working_dir'])
        self.assertEquals(self.hids[1],  prog['host_id'])
        self.assertEquals(self.hosts[1], prog['host_name'])
        self.assertEquals(self.args[1],  prog['args'])

    #-------------------------------------------------------------------------
    #  Tests for the isConsumer method.
    #
    
    ##
    # test_isConsumer_not
    
    def test_isConsumer_not(self):
        self.assertFalse(self._consumers.isConsumer(self.pids[1]))
    
    ##
    # test_isConsumer_is
    
    def test_isConsumer_is(self):
        self._consumers.add(self.rids[0], self.pids[2])
        self.assertTrue(self._consumers.isConsumer(self.pids[2]))
    
    ##
    # test_isConsumer_nosuch
    
    def test_isConsumer_nosuch(self):
        self.assertFalse(self._consumers.isConsumer(1234))
        
    #--------------------------------------------------------------------------
    #   Tests of the delete method.
    
    ##
    # test_delete_ok
    
    def test_delete_ok(self):
        id = self._consumers.add(self.rids[0], self.pids[0])
        self._consumers.delete(id)
        self.assertEquals(0, len(self._consumers.list()))
    
    ##
    # test_delete_nosuch
    
    def test_delete_nosuch(self):
        thrown = False
        try:
            self._consumers.delete(1234)
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)
    
if __name__  == '__main__':
    unittest.main()
    