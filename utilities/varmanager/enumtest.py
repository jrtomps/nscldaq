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
# @file   enumtest.py
# @brief  Test python bindings to the enumerated type.
# @author <fox@nscl.msu.edu>


import unittest
import tempfile
import os

import nscldaq.vardb.enum
import nscldaq.vardb.vardb
import nscldaq.vardb.variable

class EnumTest(unittest.TestCase):
    def setUp(self):
        myVarDb = tempfile.NamedTemporaryFile()
        myVarDbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        nscldaq.vardb.vardb.create(myVarDbName)
        self._db = nscldaq.vardb.vardb.VarDb(myVarDbName)
        self._tempFilename = myVarDbName

    def tearDown(self):
        os.remove(self._tempFilename)

    def test_create_param1isDb(self):
        with self.assertRaises(nscldaq.vardb.enum.error):
            nscldaq.vardb.enum.create('junk', 'colors', ('red', 'white', 'blue'))
            
    def test_create_param3isTuple(self):
        with self.assertRaises(nscldaq.vardb.enum.error):
            nscldaq.vardb.enum.create(self._db, 'colors', 1234)
            
    def test_create_ok(self):
        nscldaq.vardb.enum.create(self._db, 'colors', ('red', 'white', 'blue'))
        nscldaq.vardb.variable.create(self._db, None, '/myvar', 'colors')
        v = nscldaq.vardb.variable.Variable(self._db, path='/myvar')
        self.assertEquals('red', v.get())
    def test_create_dupvalue_bad(self):
        with self.assertRaises(nscldaq.vardb.enum.error):
            nscldaq.vardb.enum.create(self._db, 'colors', ('red', 'white', 'blue', 'red'))
            
    def test_id_ok(self):
        id = nscldaq.vardb.enum.create(self._db, 'colors', ('red', 'white', 'blue'))
        self.assertEquals(id, nscldaq.vardb.enum.id(self._db, 'colors'))
    
    def test_id_needsDb(self):
        with self.assertRaises(nscldaq.vardb.enum.error):
            nscldaq.vardb.enum.id('junk', 'colors')
            
    def test_id_nosuch(self):
        with self.assertRaises(nscldaq.vardb.enum.error):
            nscldaq.vardb.enum.id(self._db, 'colors')
            
    def test_list_values(self):
        id = nscldaq.vardb.enum.create(self._db, 'colors', ('red', 'white', 'blue'))
        values = nscldaq.vardb.enum.listValues(self._db, 'colors')
        self.assertTrue('red' in values)
        self.assertTrue('white' in values)
        self.assertTrue('blue' in values)
        self.assertEquals(3, len(values))
        
    def test_list_values_musthavedb(self):
        with self.assertRaises(nscldaq.vardb.enum.error):
            nscldaq.vardb.enum.listValues("not a database", 'colors')
    
    def test_list_vales_nosuchenum(self):
        with self.assertRaises(nscldaq.vardb.enum.error):
            nscldaq.vardb.enum.listValues(self._db, 'integer')          # not an enum.
    
    def test_list_enums(self):
        nscldaq.vardb.enum.create(self._db, 'colors', ('red', 'white', 'blue'))
        nscldaq.vardb.enum.create(self._db, 'stooges', ('larry', 'curly', 'moe', 'shemp'))
        nscldaq.vardb.enum.create(self._db, 'abbott-costello', ('bud', 'lou'))
        enums = nscldaq.vardb.enum.listEnums(self._db)
        self.assertTrue('colors' in enums)
        self.assertTrue('stooges' in enums)
        self.assertTrue('abbott-costello' in enums)
        self.assertEquals(3, len(enums))
        
    def test_list_enums_musthavedb(self):
        with self.assertRaises(nscldaq.vardb.enum.error):
            nscldaq.vardb.enum.listEnums('not a database')
    
    

if __name__ == '__main__':
    unittest.main()
    