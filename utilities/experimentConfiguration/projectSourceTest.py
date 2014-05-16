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
# @file   projectSourceTest.py
# @brief  Test methods of program.DataSource
# @author <fox@nscl.msu.edu>

import unittest
import project

class projectSourceTest(unittest.TestCase):
    def setUp(self):
        self._project = project.Project(":memory:")
        self._project.create()
        
        self._rings   = project.Rings(self._project)
        self._hosts   = project.Hosts(self._project)
        self._programs= project.Programs(self._project)
        self._sources = project.DataSource(self._project)

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
    #  Tests for the add method.
    
    ##
    # test_add_Ok
    #   This is an ordinary data source add that should go just fine.
    #
    def test_add_Ok(self):
        self._sources.add(self.rids[0], self.pids[0])
        
        # Ensure we can find this:
    
        conn = self._project.connection
        curs = conn.cursor()
        
        curs.execute('''
            SELECT program_id, ring_id FROM data_sources
                    ''')
        row = curs.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(self.pids[0], row[0])
        self.assertEquals(self.rids[0], row[1])
        
        row = curs.fetchone()
        self.assertEquals(None, row)
    ##
    # test_add_noSuchRing
    #   The ring must exist or there's a RuntimeError:
    
    def test_add_noSuchRing(self):
        thrown = False
        try:
            self._sources.add(1234, self.pids[0])
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)
        
    ##
    # test_add_noSuchProgram
    #   If the program id is bad, that should throw a RuntimeError too:
    #
    def test_add_noSuchProgram(self):
        thrown = False
        try:
            self._sources.add(self.rids[0], 1134)
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)
    
    ##
    # test_add_ringHasSource
    #   Not legal for two data sources to be on the same ring.
    #
    def test_add_ringHasSource(self):
        self._sources.add(self.rids[0], self.pids[0])    # Legal add
        
        pid = self._programs.add_byHostname(
            '/usr/opt/junk', '/usr/opt/daq/bin/someprogram', self.hosts[0]
        )
        thrown = False
        try:
            self._sources.add(self.rids[0], pid)
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)

    ##
    # test_add_programAlreadySource
    #    For now anyway a single program cannot be a source on more than one
    #    ring
    
    def test_add_programAlreadySource(self):
        self._sources.add(self.rids[0], self.pids[0])    # Legal add
        
        throw = False
        try:
            self._sources.add(self.rids[0], self.pids[0])
        except RuntimeError:
            throw = True
        except:
            pass
        
        self.assertTrue(throw)
    
    ##
    # test_add_wrongHost
    #   The program's host must be the same as the ring's host
    #
    def test_add_wrongHost(self):
        throw = False
        try:
            self._sources.add(self.rids[1], self.pids[0])
        except RuntimeError:
            throw = True
        except:
            pass
        
        self.assertTrue(throw)
        
    #--------------------------------------------------------------------------
    # Tsts for the addWithName method.
    #
    
    ##
    # test_addwn_ok
    #   Ok add.
    
    def test_addwn_ok(self):
        self._sources.addWithName(self.rings[1], self.pids[1])
        
        curs = self._project.connection.cursor()
        curs.execute('''
            SELECT ring_id, program_id FROM data_sources
            WHERE program_id = ?
                     ''', (self.pids[1],))
    
        row = curs.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(self.rids[1], row[0])
        self.assertEquals(self.pids[1], row[1])
        
        self.assertEquals(None, curs.fetchone())
    
    ##
    # test_addwn_noring
    #   If the ring name does not translate thrown an error:
    #
    def test_addwn_noring(self):
        thrown = False
        try:
            self._sources.addWithName('no such ring', self.pids[1])
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)
    
    #--------------------------------------------------------------------------
    #  Tests for the list method.
    #
    
    ##
    # test_list_empty
    #   Listing before adding any sources gives an empty list:
    #
    def test_list_emtpy(self):
        listing = self._sources.list()
        self.assertEquals(list(), listing)
        
    ##
    # test_list_1
    #   List with one data source
    #
    def test_list_1(self):
        self._sources.add(self.rids[2], self.pids[2])
        listing = self._sources.list()
        
        self.assertEquals(1, len(listing))
        
        self.assertEquals(1, listing[0]['id'])
        ringInfo    = listing[0]['ring_info']
        programInfo = listing[0]['program_info']
        
        self.assertEquals(self.rids[2],  ringInfo['ring_id'])
        self.assertEquals(self.rings[2], ringInfo['ring_name'])
        self.assertEquals(self.hids[2],  ringInfo['host_id'])
        self.assertEquals(self.hosts[2], ringInfo['host_name'])
        self.assertEquals(self.sourcids[2], ringInfo['sourceid'])
        
        self.assertEquals(self.pids[2],  programInfo['program_id'])
        self.assertEquals(self.paths[2], programInfo['path'])
        self.assertEquals(self.hids[2],  programInfo['host_id'])
        self.assertEquals(self.wds[2],   programInfo['working_dir'])
        self.assertEquals(self.hosts[2],      programInfo['host_name'])
        self.assertEquals(self.args[2],       programInfo['args'])
    
    ##
    #  test_list_several
    #     List several sources.
    #
    def test_list_several(self):
        for i in range(0,3):
            self._sources.add(self.rids[i], self.pids[i])
            
        listing = self._sources.list()
        self.assertEquals(3, len(listing))
        for i in range(0,3):
            ringInfo    = listing[i]['ring_info']
            programInfo = listing[i]['program_info']

            self.assertEquals(i+1, listing[i]['id'])
            self.assertEquals(self.rids[i],  ringInfo['ring_id'])
            self.assertEquals(self.rings[i], ringInfo['ring_name'])
            self.assertEquals(self.hids[i],  ringInfo['host_id'])
            self.assertEquals(self.hosts[i], ringInfo['host_name'])
            self.assertEquals(self.sourcids[i], ringInfo['sourceid'])
            
            self.assertEquals(self.pids[i],  programInfo['program_id'])
            self.assertEquals(self.paths[i], programInfo['path'])
            self.assertEquals(self.hids[i],  programInfo['host_id'])
            self.assertEquals(self.wds[i],   programInfo['working_dir'])
            self.assertEquals(self.hosts[i],      programInfo['host_name'])
            self.assertEquals(self.args[i],       programInfo['args'])            

    #--------------------------------------------------------------------------
    #  Tests for ringSource
    #
    
    
    ##
    # test_ringsource_noring
    #    ringSource returns None if there is no such ring
    
    def  test_ringsource_noring(self):
        self.assertEquals(None, self._sources.ringSource('abcdefg', 'localhost'))
    
    ##
    # test_ringsource_nosource
    #    ringSource returns None if there is no source on the ring.
    
    def test_ringsource_nosource(self):
        self.assertEquals(
            None, self._sources.ringSource(self.rings[0], self.hosts[0])
        )
        
    ##
    # test_ringsource_source
    #     ringsSource returns dict when there is a ring with a source.
    #
    def test_ringsource_source(self):
        self._sources.add(self.rids[1], self.pids[1])
        
        info = self._sources.ringSource(self.rings[1], self.hosts[1])
        
        self.assertNotEquals(None, info)
        self.assertEquals(self.sourcids[1], info['sourceid'])
        programInfo = info['program_info']
        self.assertEquals(self.pids[1],  programInfo['program_id'])
        self.assertEquals(self.paths[1], programInfo['path'])
        self.assertEquals(self.hids[1],  programInfo['host_id'])
        self.assertEquals(self.wds[1],   programInfo['working_dir'])
        self.assertEquals(self.hosts[1],      programInfo['host_name'])
        self.assertEquals(self.args[1],       programInfo['args'])
        
    #---------------------------------------------------------------------------
    #  Tests for the exists method.
    
    ##
    # test_exists_no
    
    def test_exists_no(self):
        self.assertFalse(self._sources.exists('no', 'locallhost'))
    
    ##
    # test_exists_yes
    
    def test_exists_yes(self):
        self._sources.addWithName(self.rings[1], self.pids[1])
        return self.assertTrue(self._sources.exists(self.rings[1], self.hosts[1]))
        
    #--------------------------------------------------------------------------
    # Tests for isSource
    
    ##
    # test_isSource_yes - The program id is a source

    def test_isSource_yes(self):
        self._sources.add(self.rids[2], self.pids[2])
        ringInfo = self._sources.isSource(self.pids[2])
        
        self.assertNotEquals(None, ringInfo)
        self.assertEquals(self.rids[2],  ringInfo['ring_id'])
        self.assertEquals(self.rings[2], ringInfo['ring_name'])
        self.assertEquals(self.hids[2],  ringInfo['host_id'])
        self.assertEquals(self.hosts[2], ringInfo['host_name'])
        self.assertEquals(self.sourcids[2], ringInfo['sourceid'])
                   
        
    
    ##
    # test_isSource_no - The program id is not a source.
    #
    def test_isSource_no(self):
        self.assertEquals(None, self._sources.isSource(self.pids[0]))
    ##
    # test_isSource_noprog - There is no such program id (also no).
    #
    def test_isSource_noprog(self):
        self.assertEquals(None, self._sources.isSource(1234))

    #--------------------------------------------------------------------------
    #  Tests for the delete methods
    #
    
    ##
    # test_delete_ok
    #    The delete works.
    
    def test_delete_ok(self):
        id = self._sources.add(self.rids[0], self.pids[0])
        
        self._sources.delete(id)
        self.assertFalse(self._sources.exists(self.rings[0], self.hosts[0]))
    
    ##
    # test_delete_notsource
    #   Delete nonexisting source givesa  RuntimeError:
    def test_delete_notsource(self):
        thrown = False
        try:
            self._sources.delete(1)
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)
    ##
    # test_deleteRingSource
    #
    def test_deleteRingSource(self):
        self._sources.add(self.rids[0], self.pids[0])
        self._sources.deleteRingSource(self.rids[0])
        self.assertFalse(self._sources.exists(self.rings[0], self.hosts[0]))
    
    ##
    # test_delete_named
    #
    def test_delete_named(self):
        self._sources.add(self.rids[0], self.pids[0])
        self._sources.deleteNamedRingSource(self.rings[0], self.hosts[0])
        self.assertFalse(self._sources.exists(self.rings[0], self.hosts[0]))
        

if __name__  == '__main__':
    unittest.main()