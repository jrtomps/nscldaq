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
# @file   pydstests.py
# @brief  Test suite for the data source part of the package.
# @author <fox@nscl.msu.edu>


#  This is a separate test suite so that its tests can be run independently
#  (since python tests are slow enough to impact the development cycle adversly).

import unittest
import tempfile
import os

from nscldaq.vardb import VardbEvb
from nscldaq.vardb import vardb
from nscldaq.vardb import varmgr

class TestDataSourceApi(unittest.TestCase):

    def dbInit(self):
        obj = VardbEvb.VardbEvb(self._dbUri)
        obj.createSchema()
        return obj
    
    def mkDefaultEvb(self):
        obj  = self.dbInit()
        obj.createEventBuilder('test', 'charlie', 1, 'ring')
        return obj
    
    def addDefaultDs(self):
        self._api.addDataSource('test', 'ds1', 'spdaq20',
            '/usr/opt/daq/current/bin/ringFragmentSource',
            'tcp://spdaq20/0400x', (1,))
    #
    #  Common setup requires the creation of a database and its initialization
    #  to be a variable database.
    #
    def setUp(self):
        #  This rigmarole is just to get a filename.
        
        myVarDb = tempfile.NamedTemporaryFile()
        self._dbName = myVarDb.name
        myVarDb.close()
        
        # Create/format the database:
        
        vardb.create(self._dbName)
        self._dbUri = 'file://' + self._dbName
        self._db  = varmgr.Api(self._dbUri)
        self._api = self.mkDefaultEvb()
        
    def tearDown(self):
        os.remove(self._dbName)
    
    #---------------------------------------------------------------------------
    #  The tests:
    
    def test_addDataSourceDefault(self):
        self.addDefaultDs()
        
        self._db.cd('/EventBuilder/test/ds1')
        self.assertEqual('spdaq20', self._db.get('host'))
        self.assertEqual(
            '/usr/opt/daq/current/bin/ringFragmentSource', self._db.get('path')
        )
        self.assertEqual('', self._db.get('info'))
        self.assertEqual('1', self._db.get('id0'))
        self.assertEqual('tcp://spdaq20/0400x', self._db.get('ring'))
        self.assertEqual('0', self._db.get('default-id'))
        self.assertEqual('', self._db.get('timestamp-extractor'))
        self.assertEqual('true', self._db.get('expect-bodyheaders'))
        
    def test_addDataSourceOverride(self):
        self._api.addDataSource(
            'test', 'ds1', 'spdaq20',
            '/usr/opt/daq/current/bin/ringFragmentSource',
            'tcp://spdaq20/0400x', (1,),
            info="Test data source", bodyHeaders=False, defaultId=1,
            tsextractor="/usr/bin/ls"
        )
        
        self._db.cd('/EventBuilder/test/ds1')
        
        self.assertEqual('Test data source', self._db.get('info'))
        self.assertEqual('1', self._db.get('default-id'))
        self.assertEqual('/usr/bin/ls', self._db.get('timestamp-extractor'))
        self.assertEqual('false', self._db.get('expect-bodyheaders'))
        
    def test_addDataSourceIdList(self):
        self._api.addDataSource('test', 'ds1', 'spdaq20',
            '/usr/opt/daq/current/bin/ringFragmentSource',
            'tcp://spdaq20/0400x', (1, 2))
        
        self._db.cd('/EventBuilder/test/ds1')
        
        self.assertEqual('1', self._db.get('id0'))
        self.assertEqual('2', self._db.get('id1'))

    def test_addDataSourceIdsNotiterable(self) :
        with self.assertRaises(VardbEvb.exception) :
           self._api.addDataSource('test', 'ds1', 'spdaq20',
            '/usr/opt/daq/current/bin/ringFragmentSource',
            'tcp://spdaq20/0400x', 1)
           
    def test_addDataSourceIdsNoninteger(self) :
        with self.assertRaises(VardbEvb.exception) :
           self._api.addDataSource('test', 'ds1', 'spdaq20',
            '/usr/opt/daq/current/bin/ringFragmentSource',
            'tcp://spdaq20/0400x', (1, 'string'))
    
    def test_addDataSourceExpectBodyHeaderNotBool(self):
        with self.assertRaises(VardbEvb.exception):
            self._api.addDataSource('test', 'ds1', 'spdaq20',
            '/usr/opt/daq/current/bin/ringFragmentSource',
            'tcp://spdaq20/0400x', (1, ), bodyHeaders=1)

    def test_SetHost(self):
        self.addDefaultDs()
        self._api.dsSetHost('test', 'ds1', 'charlie')
        
        self.assertEqual('charlie', self._db.get('/EventBuilder/test/ds1/host'))

    def test_SetPath(self):
        self.addDefaultDs()
        self._api.dsSetPath('test', 'ds1', '/usr/bin/ls')
        
        self.assertEqual('/usr/bin/ls', self._db.get('EventBuilder/test/ds1/path'))
        
    def test_setRingUri(self):
        self.addDefaultDs()
        self._api.dsSetRingUri('test', 'ds1', 'tcp://charlie/fox')
        
        self.assertEqual(
            'tcp://charlie/fox', self._db.get('/EventBuilder/test/ds1/ring'))
        
    def test_setInfo(self):
        self.addDefaultDs()
        self._api.dsSetInfo('test', 'ds1', 'This is an info string')
        self.assertEqual(
            'This is an info string', self._db.get('/EventBuilder/test/ds1/info')
        )
        
    def test_setDefaultId(self):
        self.addDefaultDs()
        self._api.dsSetDefaultId('test', 'ds1', 1234)
        
        self.assertEqual(
            '1234', self._db.get('/EventBuilder/test/ds1/default-id')
        )
        
    def test_noExpectBodyHeader(self):
        self.addDefaultDs()          # Expecting body headers.
        self._api.dsDontExpectBodyHeaders('test', 'ds1')
        
        self.assertEqual('false', self._db.get('/EventBuilder/test/ds1/expect-bodyheaders'))

    def test_expectBodyHeader(self):
        self.addDefaultDs()          # Expecting body headers.
        self._api.dsDontExpectBodyHeaders('test', 'ds1')   # Turn off
        self._api.dsExpectBodyHeaders('test', 'ds1')      # turn on
        
        self.assertEqual('true', self._db.get('/EventBuilder/test/ds1/expect-bodyheaders'))
        
    def test_setTimeExtractor(self):
        self.addDefaultDs()
        self._api.dsSetTimestampExtractor('test', 'ds1','/usr/bin/ls')
        
        self.assertEqual(
            '/usr/bin/ls',
            self._db.get('/EventBuilder/test/ds1/timestamp-extractor')
        )
    def test_setIds(self):
        self.addDefaultDs()
        self._api.dsSetIds('test', 'ds1', [9,8,7])
        self.assertEqual('9', self._db.get('/EventBuilder/test/ds1/id0'))
        self.assertEqual('8', self._db.get('/EventBuilder/test/ds1/id1'))
        self.assertEqual('7', self._db.get('/EventBuilder/test/ds1/id2'))
        
    def test_Info(self):
        self.addDefaultDs()
        info = self._api.dsInfo('test', 'ds1')
        
        self.assertDictEqual(
            {
                'name': 'ds1', 'host': 'spdaq20',
                'path': '/usr/opt/daq/current/bin/ringFragmentSource',   'info': '',
                'ids' : (1, ), 'ring': 'tcp://spdaq20/0400x', 'bodyheaders' : True,
                'defaultId': 0L, 'tsextractor': ''
            }, info
        )
    def test_ls(self):
        self.addDefaultDs()
        self._api.addDataSource(
            'test', 'ds2', 'charlie', '/usr/bin/ls', 'tcp://charlie/fox', (2,3),
            info='second source'
        )
        ls = self._api.listDataSources('test')
        
        info = ls[0];
        self.assertDictEqual(
            {
                'name': 'ds1', 'host': 'spdaq20',
                'path': '/usr/opt/daq/current/bin/ringFragmentSource',   'info': '',
                'ids' : (1, ), 'ring': 'tcp://spdaq20/0400x', 'bodyheaders' : True,
                'defaultId': 0L, 'tsextractor': ''
            }, info
        )
        
        info = ls[1]
        self.assertDictEqual(
            {
                'name': 'ds2', 'host': 'charlie',
                'path': '/usr/bin/ls',   'info': 'second source',
                'ids' : (2,3), 'ring': 'tcp://charlie/fox', 'bodyheaders' : True,
                'defaultId': 0L, 'tsextractor': ''
            }, info
        )
    
    def test_rm(self):
        self.addDefaultDs()
        self._api.addDataSource(
            'test', 'ds2', 'charlie', '/usr/bin/ls', 'tcp://charlie/fox', (2,3),
            info='second source'
        )
        self._api.rmDataSource('test', 'ds1')
        
        ls = self._api.listDataSources('test')
        self.assertEquals(1,len(ls))
        self.assertEquals('ds2', ls[0]['name'])
        
        
        
if __name__ == '__main__':
    unittest.main()