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
# @file   pypkgtests.py
# @brief  Test python bindings to the event builder database API.
# @author <fox@nscl.msu.edu>

import unittest
import tempfile
import os

from nscldaq.vardb import VardbEvb
from nscldaq.vardb import vardb
from nscldaq.vardb import varmgr

class TestEventBuidlerApi(unittest.TestCase):
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
        
    def tearDown(self):
        os.remove(self._dbName)
    
    def dbInit(self):
        obj = VardbEvb.VardbEvb(self._dbUri)
        obj.createSchema()
        return obj
    
    def mkDefaultEvb(self):
        obj  = self.dbInit()
        obj.createEventBuilder('test', 'charlie', 1)
        return obj
    
        
    ##########################################################################
    # Tests
    #
        
    # Instantiate an event builder API object:
    
    def test_instantiate(self):
        obj = VardbEvb.VardbEvb(self._dbUri)
    
    #  Tests for schemaExists and createSchema:
    
    def test_noschema(self):
        obj = VardbEvb.VardbEvb(self._dbUri)
        self.assertFalse(obj.schemaExists())
        
    def test_createschema(self):
        obj = self.dbInit()
        self.assertTrue(obj.schemaExists())
        
    
    # Tests for event builders themselves.
    
    def test_makeEvbDefault(self):
        obj = self.mkDefaultEvb()
        
        # There should be a directory /EventBuilder/test:
        
        self._db.cd('/EventBuilder/test')
        
        # Check the values of the event builder variables -- keeping in mind defaults.
        
        self.assertEquals('charlie', self._db.get('host'))
        self.assertEquals('ORDERER', self._db.get('servicePrefix'))
        self.assertEquals('', self._db.get('serviceSuffix'))
        self.assertEquals('1', self._db.get('coincidenceInterval'))
        self.assertEquals('true', self._db.get('build'))
        self.assertEquals('earliest', self._db.get('timestampPolicy'))
        self.assertEquals('0', self._db.get('sourceId'))

    def test_makeEvbOverrides(self):
        obj = self.dbInit()
        obj.createEventBuilder(
            'test', 'charlie', 2,
            sourceId=5, tsPolicy='latest', build=False, servicePrefix='fox',
            serviceSuffix='foxy'
        )
        
        self._db.cd('/EventBuilder/test')
        
        self.assertEquals('charlie', self._db.get('host'))
        self.assertEquals('fox', self._db.get('servicePrefix'))
        self.assertEquals('foxy', self._db.get('serviceSuffix'))
        self.assertEquals('2', self._db.get('coincidenceInterval'))
        self.assertEquals('false', self._db.get('build'))
        self.assertEquals('latest', self._db.get('timestampPolicy'))
        self.assertEquals('5', self._db.get('sourceId'))
        
    def test_makeEvbBadTsPolicy(self):
        obj = self.dbInit()
        with self.assertRaises(VardbEvb.exception) :
            obj.createEventBuilder('test', 'charlie', 2, tsPolicy='junk')
    
    def test_makeEvbBuildNotBool(self):
        obj = self.dbInit()
        with self.assertRaises(VardbEvb.exception):
            obj.createEventBuilder('test', 'charlie', 2, build=1)
        
    
    def test_setEvbHost(self):
        obj = self.mkDefaultEvb()
        obj.setEvbHost('test', 'spdaq20')
        
        self._db.cd('/EventBuilder/test')
        self.assertEquals('spdaq20', self._db.get('host'))
    
    def test_setEvbDt(self):
        obj = self.mkDefaultEvb()
        obj.setEvbCoincidenceInterval('test', 2)
        
        self._db.cd('/EventBuilder/test')
        self.assertEquals('2', self._db.get('coincidenceInterval'))
     
    def test_setEvbSourceId(self):
        obj = self.mkDefaultEvb()
        obj.setEvbSourceId('test', 666)

        self._db.cd('/EventBuilder/test')
        self.assertEquals('666', self._db.get('sourceId'))
    
    def test_setEvbSvcPrefix(self):
        obj = self.mkDefaultEvb()
        obj.setEvbServicePrefix('test', 'ron')
        
        self._db.cd('/EventBuilder/test')
        self.assertEquals('ron', self._db.get('servicePrefix'))
        
    def test_diablebuild(self):
        obj = self.mkDefaultEvb()     #Enabled.
        obj.disableEvbBuild('test')
        
        self._db.cd('/EventBuilder/test')
        self.assertEquals('false', self._db.get('build'))
    
    def test_enablebuild(self):
        obj = self.mkDefaultEvb()
        obj.disableEvbBuild('test')    #Disabled.
        obj.enableEvbBuild('test')     #reenabled.
        
        self._db.cd('/EventBuilder/test')
        self.assertEquals('true', self._db.get('build'))
    
    def test_setTimestampPolicy(self):
        obj = self.mkDefaultEvb()
        
        self._db.cd('/EventBuilder/test')
        
        obj.setEvbTimestampPolicy('test', 'latest')
        self.assertEquals('latest', self._db.get('timestampPolicy'))
        
        obj.setEvbTimestampPolicy('test', 'average')
        self.assertEquals('average', self._db.get('timestampPolicy'))
        
        obj.setEvbTimestampPolicy('test', 'earliest')
        self.assertEquals('earliest', self._db.get('timestampPolicy'))

        with self.assertRaises(VardbEvb.exception):
            obj.setEvbTimestampPolicy('test', 'junk')
            
    def test_setServiceSuffix(self):
        obj = self.mkDefaultEvb()
        obj.setEvbServiceSuffix('test', 'fox')

        self._db.cd('/EventBuilder/test');
        self.assertEqual('fox', self._db.get('serviceSuffix'))
        
    def test_rmEvb(self):
        obj = self.mkDefaultEvb()
        obj.rmEventBuilder('test')
        
        with self.assertRaises(varmgr.error) :
            self._db.cd('/EventBuilder/test')
    
        
if __name__ == '__main__':
    unittest.main()

