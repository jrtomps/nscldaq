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
# @file   variabletest.py
# @brief  Test for nscldaq.vardb.variable module -- static and object methods.
# @author <fox@nscl.msu.edu>

import unittest
import nscldaq.vardb.vardb
import nscldaq.vardb.dirtree
import nscldaq.vardb.variable

import tempfile
import os
import sqlite3



class variableTest(unittest.TestCase):
    def setUp(self):
        myVarDb = tempfile.NamedTemporaryFile()
        myVarDbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        nscldaq.vardb.vardb.create(myVarDbName)
        self._db = nscldaq.vardb.vardb.VarDb(myVarDbName)
        self._dir= nscldaq.vardb.dirtree.DirTree(self._db)
        self._tempFilename = myVarDbName
        
    def tearDown(self):
        os.remove(self._tempFilename)

    def getVarInfo(self, varname):
        conn = sqlite3.connect(self._tempFilename)
        curs = conn.cursor()
        curs.execute(
            '''SELECT v.name, v.value, v.directory_id, t.type_name
            FROM variables v
            INNER JOIN variable_types t ON t.id = v.type_id
            WHERE name = ? ''', (varname,)
        )
        row = curs.fetchone()
        self.assertIsNotNone(row)
        conn.close()
        return {'name': row[0], 'value': row[1], 'dirid': row[2], 'type': row[3]}
        
        
    def test_dummy(self):
        pass

    # First parameter of create must be a database.

    def test_create_nodbobj(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.create(1, None, "/this/is/a/test", "integer")

    # Create without a directory object (OK)

    def test_create_nodirobj(self):
        nscldaq.vardb.variable.create(self._db, None, "/myvar", "integer")
        result = self.getVarInfo('myvar')
        self.assertEquals("myvar", result['name'])
        self.assertEquals("0", result['value'])                   # Default integer value
        self.assertEquals(self._dir.getRootId(), result['dirid'])
        self.assertEquals('integer', result['type'])

    # Create with a directory object (OK)

    def test_create_dirobj(self):
        self._dir.mkdir("/this/is/a/test/directory")
        self._dir.cd("/this/is/a")
        nscldaq.vardb.variable.create(self._db, self._dir, "test/directory/myvar", "integer")
        self._dir.cd("test/directory")
        correctDirId = self._dir.getwd()['id']
        
        result = self.getVarInfo('myvar')
        self.assertEquals("myvar", result['name'])
        self.assertEquals("0", result['value'])                   # Default integer value
        self.assertEquals(correctDirId, result['dirid'])
        self.assertEquals('integer', result['type'])

    # Create with a bad directory object
    
    def test_create_baddirobj(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.create(self._db, 1234, "/myvar", "integer")
    
    # Create with a bad path
    
    def test_create_badpath(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.create(self._db, None, "/this/does/not/exist/myvar", "integer")
    
    # Create with a good initial value.
    
    def test_create_good_initial(self):
        nscldaq.vardb.variable.create(self._db, None, "/myvar", "integer", '1234')
        result = self.getVarInfo('myvar')
        self.assertEquals('1234', result['value'])
    
    # Create wit a bad initial value
        
    def test_create_bad_initial(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.create(self._db, None, "/myvar", "integer", '1234five')
    
    # Test ls with bad db
    
    def test_ls_baddb(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.ls(1234, 1234)
    
    # Test ls with bad dir.
    def test_ls_baddir(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.ls(self._db, 1234)
    
    # Test ls with default path, empty directory
    
    def test_ls_emptydir(self):
        self._dir.cd('/')
        result = nscldaq.vardb.variable.ls(self._db, self._dir)
        self.assertEqual([], result)
    
    # test ls with default path, some items.
    
    def test_ls_nonempty(self):
        self._dir.mkdir('/test/directory')
        self._dir.cd('/test/directory')
        files = ['atest', 'btest', 'cdone']
        for path in files:
            nscldaq.vardb.variable.create(self._db, self._dir, path, "integer")
        
        listing = nscldaq.vardb.variable.ls(self._db, self._dir)
        self.assertEquals(len(files), len(listing))
        
        dirid = self._dir.getwd()['id']
        
        # Supposedly everything is sorted:
        
        for i in range(len(files)):
            self.assertEqual(files[i],  listing[i]['name'])
            self.assertEqual('integer', listing[i]['type'])
            self.assertEqual(dirid,     listing[i]['dirId'])
            
    
    # test ls with relative path, some items.
    
    def test_ls_relpath(self):
        self._dir.mkdir('/test/directory/here')
        self._dir.cd('/test/directory')
        files = ['atest', 'btest', 'cdone']
        for path in files:
            nscldaq.vardb.variable.create(
                self._db, self._dir, '/'.join(('here', path)), "integer"
            )
        listing = nscldaq.vardb.variable.ls(self._db, self._dir, 'here')
        self.assertEquals(len(files), len(listing))
        self._dir.cd('here')
        dirid = self._dir.getwd()['id']
        
        # Supposedly everything is sorted:
        
        for i in range(len(files)):
            self.assertEqual(files[i],  listing[i]['name'])
            self.assertEqual('integer', listing[i]['type'])
            self.assertEqual(dirid,     listing[i]['dirId'])
            
    #  Set with legal value;
    
    def test_set_legalvalue(self):
        nscldaq.vardb.variable.create(self._db, None, '/myvar', 'integer')
        variable = nscldaq.vardb.variable.Variable(self._db, path='/myvar')
        variable.set('1234')
        varinfo = self.getVarInfo('myvar')
        self.assertEquals('1234', varinfo['value'])
    
    # Set with illegal value;
    
    def test_set_illegalvalue(self):
        nscldaq.vardb.variable.create(self._db, None, '/myvar', 'real')
        variable = nscldaq.vardb.variable.Variable(self._db, path='/myvar')
        
        with self.assertRaises(nscldaq.vardb.variable.error):
            variable.set('1.234five')
    
    # Test get:
    
    def test_get(self):
        nscldaq.vardb.variable.create(self._db, None, '/myvar', 'real', '3.14159')
        variable = nscldaq.vardb.variable.Variable(self._db, path='/myvar')
        self.assertEqual('3.14159', variable.get())
        
    # Test getName:
    
    def test_getName(self):
        nscldaq.vardb.variable.create(self._db, None, '/myvar', 'real', '3.14159')
        variable = nscldaq.vardb.variable.Variable(self._db, path='/myvar')
        self.assertEqual('myvar', variable.getName())    
    
    # Test getDirectory not root.
    
    def test_getDir_notroot(self):
        vardir = '/some/subdir/path'
        variablePath = '/'.join((vardir, 'myvar'))
        self._dir.mkdir(vardir)
        nscldaq.vardb.variable.create(self._db, None, variablePath, 'real', '3.14159')
        variable = nscldaq.vardb.variable.Variable(self._db, path=variablePath)
        self.assertEqual(vardir, variable.getDirectory())
    
    
    # Test getDirectory root.
    
    def test_getDir_root(self):
        nscldaq.vardb.variable.create(self._db, None, '/myvar', 'integer')
        variable = nscldaq.vardb.variable.Variable(self._db, path='/myvar')
        self.assertEqual('/', variable.getDirectory())
    
    
 
 # Variable destruction is pretty complex due to the overloads so we're going
 # to do that in a separate test class:
 
class VariableDestroy(unittest.TestCase):
    def setUp(self):
        myVarDb = tempfile.NamedTemporaryFile()
        myVarDbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        nscldaq.vardb.vardb.create(myVarDbName)
        self._db = nscldaq.vardb.vardb.VarDb(myVarDbName)
        self._dir= nscldaq.vardb.dirtree.DirTree(self._db)
        self._tempFilename = myVarDbName
        
    def tearDown(self):
        os.remove(self._tempFilename)
    
    #   Tests for destroy by id:
    
    #  Whacky first parameter is no good:
    
    def test_bad_dbfirst(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.destroy('junk', 1)
    #  Delete by good id
    
    def test_delete_byid_ok(self):
        nscldaq.vardb.variable.create(self._db, None, "/myvar", "integer")
        listing = nscldaq.vardb.variable.ls(self._db, self._dir)
        varid   = listing[0]['id']
        
        nscldaq.vardb.variable.destroy(self._db, varid)
        after   = nscldaq.vardb.variable.ls(self._db, self._dir)
        self.assertEquals(0, len(after))
    
    #  Delete by bad id.
    
    def test_deleted_byid_bad(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.destroy(self._db, 12345)
            
    # Delete by abspath ok.
    
    def test_delete_byabspath_ok(self):
        dirpath='/this/is/a/path'
        self._dir.mkdir(dirpath)

        nscldaq.vardb.variable.create(self._db, None, '/'.join((dirpath, 'myvar')), 'integer')
        
        nscldaq.vardb.variable.destroy(self._db, '/'.join((dirpath, 'myvar')))
        self._dir.cd(dirpath)
        listing = nscldaq.vardb.variable.ls(self._db, self._dir)
        self.assertEquals(0, len(listing))
                                      
    
    # Delete by abspath bad.
    
    def test_delete_byabspath_bad(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.destroy(self._db, '/this/that/theother/myvar')
    
    # Delete by relpath ok
    
    def test_delete_byrelpath_ok(self):
        dirpath='/this/is/a/path'
        self._dir.mkdir(dirpath)

        nscldaq.vardb.variable.create(self._db, None, '/'.join((dirpath, 'myvar')), 'integer')
        self._dir.cd(dirpath)
        
        nscldaq.vardb.variable.destroy(self._db, self._dir, 'myvar')
        
        listing = nscldaq.vardb.variable.ls(self._db, self._dir)
        self.assertEquals(0, len(listing))
    
    # delete by relpath bad.
    
    def test_delete_byrelpath_bad(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            nscldaq.vardb.variable.destroy(self._db, self._dir, 'myvar')


    # Tests for variable construction - note that we also need the getId method
    # to be working:
    
class VariableConstruction(unittest.TestCase):
    def setUp(self):
        myVarDb = tempfile.NamedTemporaryFile()
        myVarDbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        nscldaq.vardb.vardb.create(myVarDbName)
        self._db = nscldaq.vardb.vardb.VarDb(myVarDbName)
        self._dir= nscldaq.vardb.dirtree.DirTree(self._db)
        self._tempFilename = myVarDbName
        
        dirname = '/this/that/theother'
        self._dirpath = dirname
        self._varname = 'myvarl'
        self._varpath = '/'.join((dirname, self._varname))
        
        
        self._dir.mkdir(dirname)
        nscldaq.vardb.variable.create(
            self._db, None, self._varpath, 'integer', '1234'
        )
        tempdir = nscldaq.vardb.dirtree.DirTree(self._db)
        tempdir.cd(dirname)
        self._varid = nscldaq.vardb.variable.ls(self._db, tempdir)[0]['id']
        
    def tearDown(self):
        os.remove(self._tempFilename)        
    
    #  Need db parameter.  Fail detected by ParseTupleAndKeywords
    
    def test_needdb(self):
        with self.assertRaises(TypeError):
            variable = nscldaq.vardb.variable.Variable(id=self._varid)
    
    #  dir and id are incompatible.
    
    def test_dirandid_incompatible(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            variable = nscldaq.vardb.variable.Variable(self._db, dir=self._dir, id=self._varid)
    
    #  path and id are incompatible.
    
    def test_pathandid_incompatible(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            variable = nscldaq.vardb.variable.Variable(self._db, path='/this/path', id=self._varid)
    
    #  must have one of path and id
    
    def test_oneof_pathandid_required(self ):
        with self.assertRaises(nscldaq.vardb.variable.error):
            variable = nscldaq.vardb.variable.Variable(self._db)
            
    #  Construct by id ok.
    
    def test_byid_ok(self):
        variable = nscldaq.vardb.variable.Variable(self._db, id=self._varid)
        self.assertEquals(self._varid, variable.getId())
    
    
    #  Construct by id fails.
    
    def test_byid_fails(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            variable = nscldaq.vardb.variable.Variable(self._db, id=self._varid+1)
    
    # Construct by abspath works.
    
    def test_construct_abspath_ok(self):
        variable = nscldaq.vardb.variable.Variable(self._db, path=self._varpath)
        self.assertEquals(self._varid, variable.getId())
    
    # Construct by abspath fails (assumes /no/such/path/myvar really doesn't exist).
    
    def test_construct_abspath_fails(self):
        with self.assertRaises(nscldaq.vardb.variable.error):
            variable = nscldaq.vardb.variable.Variable(self._db, path='/no/such/path/myvar')
    
    def test_construct_relpath_ok(self):
        self._dir.cd(self._dirpath)
        variable = nscldaq.vardb.variable.Variable(self._db, path=self._varname, dir=self._dir)
        self.assertEquals(self._varid, variable.getId())
        
    def test_construct_relpath_bad(self):
        self._dir.cd(self._dirpath)
        with self.assertRaises(nscldaq.vardb.variable.error):
            variable = nscldaq.vardb.variable.Variable(self._db, dir=self._dir, path='novar')

if __name__ == '__main__':
    unittest.main()
