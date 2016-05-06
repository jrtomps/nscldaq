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
# @file   pyapitests.py
# @brief  Test the python bindings for the ring buffer database API.
# @author <fox@nscl.msu.edu>

import unittest
import tempfile
import os

from  nscldaq.vardb import VardbRingbuffer
from  nscldaq.vardb import vardb
from  nscldaq.vardb import varmgr

class TestRingApi(unittest.TestCase):
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
    
    ##
    #  Most tests will use this to get an api built and init the database.
    #
    def dbInit(self):
        api = VardbRingbuffer.api(self._dbUri)
        api.createSchema()
        return api
    
    def ringInit(self):
        api = self.dbInit()
        api.create('fox', 'charlie')
        return api
        
    def test_noschema(self):
        api = VardbRingbuffer.api(self._dbUri)
        self.assertFalse(api.haveSchema())
    
    def test_makeSchema(self):
        api = VardbRingbuffer.api(self._dbUri)
        api.createSchema()
        self.assertTrue(api.haveSchema())
    
    def test_makeRingDefaults(self):
        api = self.ringInit()
        
        # Make sure our dir was created:
        
        dirs = self._db.ls('/RingBuffers')
        self.assertEqual('fox@charlie', dirs[0])
        self._db.cd('/RingBuffers/fox@charlie')
        self.assertEqual(str(8*1024*1024), self._db.get('datasize'))
        self.assertEqual(str(100), self._db.get('maxconsumers'))
    
    def test_makeRingOverideDefaults(self):
        api = self.dbInit()
        api.create('fox', 'charlie', datasize=1024*1024, maxconsumers=10)
        
        dirs = self._db.ls('/RingBuffers')
        self.assertEqual('fox@charlie', dirs[0])
        self._db.cd('/RingBuffers/fox@charlie')
        self.assertEqual(str(1024*1024), self._db.get('datasize'))
        self.assertEqual(str(10), self._db.get('maxconsumers'))
    
    def test_destroyDef(self):
        api = self.ringInit()
        api.destroy('fox', 'charlie')
        
        dirs = self._db.ls('/RingBuffers')
        self.assertEqual(0, len(dirs))               # sb no directories now.
        
    def test_setDataSize(self):
        api = self.ringInit()
        api.setMaxData('fox', 'charlie', 1024*1024)
        
        self._db.cd('/RingBuffers/fox@charlie')
        self.assertEqual(str(1024*1024), self._db.get('datasize'))
        
    def test_setMaxConsumers(self):
        api = self.ringInit()
        api.setMaxConsumers('fox', 'charlie', 25);
        self._db.cd('/RingBuffers/fox@charlie')
        self.assertEqual(str(25), self._db.get('maxconsumers'))
        
    def test_ringInfo(self):
        api = self.ringInit();
        info = api.ringInfo('fox', 'charlie')
        
        self.assertEqual('fox', info['name'])
        self.assertEqual('charlie', info['host'])
        self.assertEqual(100, info['maxconsumers'])
        self.assertEqual(8*1024*1024, info['datasize'])
    
    def test_list(self):
        api = self.ringInit()                   # Makes fox@charlie
        api.create('aring', 'spdaq20', datasize=2*1024*1024, maxconsumers=50);
        
        listing = api.list()
        self.assertEqual(2, len(listing))
        info = listing[0]
        self.assertEqual('aring', info['name'])
        self.assertEqual('spdaq20', info['host'])
        self.assertEqual(50, info['maxconsumers'])
        self.assertEqual(2*1024*1024, info['datasize'])
        
        info = listing[1]
        self.assertEqual('fox', info['name'])
        self.assertEqual('charlie', info['host'])
        self.assertEqual(100, info['maxconsumers'])
        self.assertEqual(8*1024*1024, info['datasize'])
        
    def test_setPos(self):
        api = self.ringInit()
        api.setEditorPosition('fox', 'charlie', 200, 300)
        
        self.assertEqual(str(200), self._db.get('/RingBuffers/fox@charlie/editorx'))
        self.assertEqual(str(300), self._db.get('/RingBuffers/fox@charlie/editory'))
        
    def test_getXpos(self):
        api = self.ringInit()
        api.setEditorPosition('fox', 'charlie', 200, 300)
        
        self.assertEqual(200, api.getEditorXPosition('fox', 'charlie'))
        
    def test_getYpos(self):
        api = self.ringInit()
        api.setEditorPosition('fox', 'charlie', 200, 300)
        
        self.assertEqual(300, api.getEditorYPosition('fox', 'charlie'))

if __name__ == '__main__':
    unittest.main()



