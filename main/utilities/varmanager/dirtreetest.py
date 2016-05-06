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
# @file   dirtreetest.py
# @brief  Tests for the VarDirTree python class.
# @author <fox@nscl.msu.edu>


import unittest
import sqlite3
import tempfile
import nscldaq.vardb.vardb
import nscldaq.vardb.dirtree
import os


class dirtreeTest(unittest.TestCase) :
    def setUp(self):
        myVarDb = tempfile.NamedTemporaryFile()
        myVarDbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        nscldaq.vardb.vardb.create(myVarDbName)
        self._db = nscldaq.vardb.vardb.VarDb(myVarDbName)
        self._tempFilename = myVarDbName
    def tearDown(self):
        os.remove(self._tempFilename)
        
    # Utility methods:
    
    ##
    # pathInfo - use sqlite to get the info about a path.
    #  @param path - the path.
    #  @return mixed
    #  @retval None - no such path.
    #  @retval Dict - info containing:
    #                 - 'name'  - name of final element of the path.
    #                 - 'id'    - Id of the node in the directory table.
    #                 - 'parent'- Id of the parent of the node (0 if root).
    def pathInfo(self, path):
        conn      = sqlite3.connect(self._tempFilename)
        pathTuple = path.split('/')[1:]
        result    = {'name':'', 'id':1, 'parent':0}    # root info.
    
        sql = '''SELECT id, name, parent_id FROM directory
                    WHERE parent_id = ? AND name =?'''
        
        for pathEle in pathTuple:
            curs = conn.cursor()
            curs.execute(sql, (result['id'], pathEle))
            result = curs.fetchone()
            if result == None:
                return result
            result = {'id':result[0], 'name':result[1], 'parent':result[2]}
        return result
    
    ##
    # havePath - use sqlite to verify the existence of a path.
    # @param path - absolute path.
    # @return boolean true  if path exists, false otherwise.
    #
    def havePath(self, path):
        return self.pathInfo(path) != None
    
        
    # test static methods
    
    def test_pathparse(self):
        path = '/this/is/a/path'
        result = nscldaq.vardb.dirtree.parsePath(path);
        self.assertEquals(path.split('/')[1:], result)
    def test_isrelative(self):
        self.assertTrue(nscldaq.vardb.dirtree.isRelative('relative/path'))
    def test_notrelative(self):
        self.assertFalse(nscldaq.vardb.dirtree.isRelative('/absolute/path'))
        
    # test construction
    
    def test_constructok(self):
        db = nscldaq.vardb.dirtree.DirTree(self._db)  
    def test_constructnotdb(self):
        with self.assertRaises(nscldaq.vardb.dirtree.error):
            db = nscldaq.vardb.dirtree.DirTree('Not A database')
    def test_constructnoarg(self):
        with self.assertRaises(nscldaq.vardb.dirtree.error):
            db = nscldaq.vardb.dirtree.DirTree()
    
    #  getRootId:
    
    def test_getRootid(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        id  = dir.getRootId()
        self.assertEquals(1, id)
        
    # mkdir
    
    def test_mkdir_abs(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        path = '/this/that/the/other'
        dir.mkdir(path)
        self.assertTrue(self.havePath(path))
    
    def test_mkdir_rel(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        relpath = 'this/that/the/other'
        abspath = '/' + relpath
        dir.mkdir(relpath)
        self.assertTrue(self.havePath(abspath))
    
    def test_mkdir_nopath_ok(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        path1 = '/this/that'
        path2 = '/this/that/theother'
        dir.mkdir(path1)
        dir.mkdir(path2, False)
        self.assertTrue(self.havePath(path2))
        
    def test_mkdir_nopath(self):
        dir  = nscldaq.vardb.dirtree.DirTree(self._db)
        path = '/this/that/the/other/'
        with self.assertRaises(nscldaq.vardb.dirtree.error):
            dir.mkdir(path, False)
        self.assertFalse(self.havePath(path))

    def test_mkdir_badargcounts(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        with self.assertRaises(nscldaq.vardb.dirtree.error):
            dir.mkdir()       
    
    def test_mkdir_nonboolflag(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        with self.assertRaises(nscldaq.vardb.dirtree.error):
            dir.mkdir('/this/is/ok', ['this', 'is', 'not'])

    def test_mkdir_duppath(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        path = '/this/is/a/path'
        dir.mkdir(path)              #ok
        with self.assertRaises(nscldaq.vardb.dirtree.error):
            dir.mkdir(path)          # dup.
    
    # cd/wdPath
    
    def test_cd_okabs(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        path = '/this/is/a/path'
        dir.mkdir(path)
        dir.cd(path)
        self.assertEqual(path, dir.wdPath())
        
        
    def test_cd_okrel(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        path = '/this/is/a/path'
        dir.mkdir(path)
        dir.cd('/this')
        dir.cd('is/a/path')
        self.assertEqual(path, dir.wdPath())
        
    def test_cd_nopath(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        with self.assertRaises(nscldaq.vardb.dirtree.error):
            dir.cd('/does/not/exist')
            
    def test_cd_aboveroot(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        with self.assertRaises(nscldaq.vardb.dirtree.error):
            dir.cd('..')
    
    #  getwd
    
    def test_getwd(self):
        path = '/subdir'
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        dir.mkdir(path)
        dir.cd(path)
        info = dir.getwd()
        self.assertEqual(self.pathInfo(path), info)
        
    # ls
    
    def test_ls_empty(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        self.assertEqual((), dir.ls())
        
    def test_ls_top(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        dirs = ('one', 'two', 'three')
        for item in dirs:
            dir.mkdir(item)
        dirs = sorted(dirs)
        self.assertEqual(dirs, list(dir.ls()))
    def test_ls_cd(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        dir.mkdir('adir')
        dir.cd('adir')
        dirs = ('one', 'two', 'three')
        for item in dirs:
            dir.mkdir(item)
        dirs = sorted(dirs)
        self.assertEqual(dirs, list(dir.ls()))
        
    #rmdir
    
    def test_rmdir_ok(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        dir.mkdir('atest');
        dir.rmdir('atest')
        self.assertEqual((), dir.ls())
        
    def test_rmdir_nox(self):
        dir = nscldaq.vardb.dirtree.DirTree(self._db)
        with self.assertRaises(nscldaq.vardb.dirtree.error):
            dir.rmdir('atest')

if __name__ == '__main__':
    unittest.main()

