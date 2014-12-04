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
# @file   test_vardb.py
# @brief  Test python bindings to the vardb module.
# @author <fox@nscl.msu.edu>


import unittest
import nscldaq.vardb.vardb
import sqlite3
import tempfile


class vardbTest(unittest.TestCase) :
    def setUp(self):
        myVarDb = tempfile.NamedTemporaryFile()
        myVarDbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        self._dbName = myVarDbName

    #  test that vardb.create('filename') produces an sqlite3 database with
    #  a directory table.
    
    def test_create(self):
        
        #  Create the variable database:
        
        nscldaq.vardb.vardb.create(self._dbName)
        
        # open as an sqlite db and check for the directory table:
        
        conn = sqlite3.connect(self._dbName)
        c    = conn.cursor()
        c.execute('''SELECT COUNT(*) FROM sqlite_master WHERE TYPE='table' AND name='directory' ''')
        row = c.fetchone()
        c.close()
        self.assertEqual(1, row[0])
    
    #  Test that we can successfully instantiate a VarDb type with a good database
    #  name.
    
    def test_instantiate_ok(self):
        nscldaq.vardb.vardb.create(self._dbName)
        try:
            item = nscldaq.vardb.vardb.VarDb(self._dbName)
            self.assertTrue(True)
        except:
            self.assertTrue(False)
    #
    #  On the other hand, constructing with a file that is not a db should throw the error exception
    #  fromthat module.
    def test_instantiate_fail(self):
        myVarDb = tempfile.NamedTemporaryFile()
        myVarDbName = myVarDb.name
        with self.assertRaises(nscldaq.vardb.vardb.error):
            item = nscldaq.vardb.vardb.VarDb(myVarDbName)
        
    
    
if __name__ == '__main__':
    unittest.main()