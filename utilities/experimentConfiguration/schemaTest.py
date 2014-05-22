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
# @file   schemaTest.py
# @brief  Test schema creation etc. for projects.
# @author <fox@nscl.msu.edu>

import project
import unittest

class TestProject(unittest.TestCase):
    # Utility code:
    
    ##
    # check_table
    #   assert the presence of a specific table:
    def check_table(self, tableName):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute(
            'SELECT COUNT(*) FROM sqlite_master WHERE type="table" AND name=?',
            (tableName,)
        )
        self.assertEquals((1,), curs.fetchone())        
    
    # Common test code:
    
    def setUp(self):
        self._project = project.Project(':memory:')
        self._project.create()
        
    def tearDown(self):
        pass
    
    # The tests:
    
    ##
    # test_create_nodes
    #   Creating a project should give us a nodes table:
    #
    def test_create_hosts(self):
        self.check_table('hosts')
    ##
    # test_create_rings
    #    Creating  a project should give us a rings table:
    #
    def test_create_rings(self):
        self.check_table('rings')
    ##
    # test_create_programs
    #
    def test_create_programs(self):
        self.check_table('programs')
    ##
    # test_create_program_args
    #
    def test_create_program_args(self):
        self.check_table('program_args')
    ##
    # test_create_data_sources
    #   This is a join table that defines which programs are data sources
    #   for which rings -- the ring itself is now holding the data sourceid.
    #
    def test_create_data_sources(self):
        self.check_table('data_sources')
    ##
    # test_create_consumers
    #   Join table that defines which programs consume from which rings.
    
    def test_create_consumers(self):
        self.check_table('consumers')
    ##
    # test_create_eventbuilder_sources
    #  Join table between data source programs and a program
    #  that feeds data into the eventbuilder.
    #
    def test_create_eventbuilder_sources(self):
        self.check_table('evb_datasources')
    ##
    # test_create_loggers
    #   Loggers are event log programs.
    #
    def test_create_loggers(self):
        self.check_table('loggers')
    ##
    # test_create_eventbuilder
    #
    #  The event builder is a singleton program that's bound to a host
    #  with an output ring (a special data aource basically).
    #
    def test_create_eventbuilder(self):
        self.check_table('eventbuilder')
    ##
    # test_create_statemanager
    #   There is also a singleton statemanager (0MQ publisher of state).
    #
    def test_create_statemanager(self):
        self.check_table('statemanager')
        
        
if __name__ == '__main__':
    unittest.main()