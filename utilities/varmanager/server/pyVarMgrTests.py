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
# @file   pyVarMgrTests.py
# @brief  Unit tests for the varmgr bindings to python.
# @author <fox@nscl.msu.edu>

import unittest
import os
import os.path
import tempfile

import nscldaq.vardb.varmgr
import nscldaq.vardb.vardb
import nscldaq.vardb.dirtree
import nscldaq.vardb.variable

class VarMgrCreationTests(unittest.TestCase):
    #
    #  Setup - figure out a good name for the database file
    #          and create a URI to it.
    def setUp(self ):
        db = tempfile.NamedTemporaryFile()
        self._dbName = db.name
        db.close()
        self._uri    = 'file://' + self._dbName

    def tearDown(self):
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
    #
    # Create creates something that could be used for database access.
    #
    def test_create(self):
        nscldaq.vardb.varmgr.create(self._dbName)
        db = nscldaq.vardb.vardb.VarDb(self._dbName)
    
    def test_instantiate(self):
        nscldaq.vardb.varmgr.create(self._dbName)
        api = nscldaq.vardb.varmgr.Api(self._uri)


class VarmgrApiTests(unittest.TestCase):
    def setUp(self ):
        db = tempfile.NamedTemporaryFile()
        self._dbName = db.name
        db.close()
        self._uri    = 'file://' + self._dbName
        nscldaq.vardb.varmgr.create(self._dbName)
        self._api = nscldaq.vardb.varmgr.Api(self._uri)
        self._db  = nscldaq.vardb.vardb.VarDb(self._dbName)
        self._dir = nscldaq.vardb.dirtree.DirTree(self._db)

    def tearDown(self):
        if os.path.isfile(self._dbName):
            os.unlink(self._dbName)
    #
    # Make absolute path
    #
    def test_mkdirAbs(self):
        self._api.mkdir('/test')
        dirs = self._dir.ls()
        
        self.assertEquals(1, len(dirs))
        self.assertEquals('test', dirs[0])
    #error:
    def test_mkdirNoPath(self):
        try:
            self._api.mkdir()
        except:
            self.assertTrue(True)
            return
        self.assertFalse(False)
    
    # cd tests - they revolve around using mkdir with relative paths.
    
    def test_cdNosuch(self):
        self.assertRaises(nscldaq.vardb.varmgr.error, self._api.cd, 'nosuch')
    def test_cdOk(self):
        self._api.mkdir('/level1')
        self._api.cd('level1')
        self._api.mkdir('level2')        # /level1/level2
        
        self._dir.cd('/level1')
        dirs = self._dir.ls()
        
        self.assertEquals(1, len(dirs))
        self.assertEquals('level2', dirs[0])
    
    def test_mkdirAbsPath(self):
        self._api.mkdir('/level1')
        self._api.cd('/level1')
        self._api.mkdir('/anotherdir')
        
        dirs  = self._dir.ls()
        self.assertEquals(2, len(dirs))
        dirs = sorted(dirs)
        self.assertEquals('anotherdir', dirs[0])
        self.assertEquals('level1', dirs[1])
    
    def test_cdRelative(self):
        self._api.mkdir('/level1/level2/level3')
        self._api.cd('/level1/level2/level3')
        self._api.mkdir('/level1/test')
        self._api.cd('../../test')
        self._api.mkdir('junk')
        
        self._dir.cd('level1/test')
        dirs = self._dir.ls()
        self.assertEquals(1, len(dirs))
        self.assertEquals('junk', dirs[0])
        
    def test_getwd(self):
        self._api.mkdir('/level1/level2/level3')
        self._api.cd('/level1/level2');
        self.assertEqual('/level1/level2', self._api.getwd())
    
    # Getwd should not have parameters.
    #
    def test_getwdWithParam(self):
        self.assertRaises(nscldaq.vardb.varmgr.error, self._api.getwd, 'test')
        
    # rmdir - nosuch
    
    def test_rmdirNosuch(self):
        self.assertRaises(nscldaq.vardb.varmgr.error, self._api.rmdir, 'test')
    
    # rmdir - abs with cd other than where it is.
    
    def test_rmdirAbs(self):
        self._api.mkdir('/level1/level2/level3')
        self._api.cd('/level1/level2/level3')
        self._api.mkdir('/test')
        self._api.rmdir('/test')
        
        self._api.cd('/')
        dirs = self._dir.ls()
        
        self.assertEquals(1, len(dirs))
        self.assertEquals('level1', dirs[0])
    
    # rmdir - rel
    
    def test_rmdirRel(self):
        self._api.mkdir('/level1/level2/level3')
        self._api.cd('/level1/level2')
        
        self._api.rmdir('level3')
        self._dir.cd('level1/level2')
        dirs = self._dir.ls()
        
        self.assertEquals(0, len(dirs))
        
    # declare ok
    
    def test_declareOk(self):
        self._api.declare('/intvar', 'integer')
        
        var = nscldaq.vardb.variable.Variable(self._db, path='/intvar')
        self.assertEquals('0', var.get())
    
    # declare relpath
    
    def test_declareRelpath(self):
        self._api.mkdir('/level1/level2')
        self._api.cd('/level1')
        self._api.declare('level2/anint', 'integer')
        
        var = nscldaq.vardb.variable.Variable(self._db, path='/level1/level2/anint')
        self.assertEquals('0', var.get())
        
    # declare abspath
    
    def test_declareAbspath(self):
        self._api.mkdir('/level1/level2')
        self._api.cd('/level1')
        self._api.declare('/level1/level2/anint', 'integer')
        
        var = nscldaq.vardb.variable.Variable(self._db, path='/level1/level2/anint')
        self.assertEquals('0', var.get())
        
    # declare bad path
    
    def test_declareBadPath(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.declare('/level1/myvar', 'integer')
    
    #  declare bad type
    
    def test_declareBadType(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.declare('/junk', 'junk')
            
    # declare initial value
    
    def test_declareInitialValue(self):
        self._api.declare('/aninteger', 'integer', '1234')
        var = nscldaq.vardb.variable.Variable(self._db, path='/aninteger')
        self.assertEquals('1234', var.get())
    
    # declare missing type.
    
    def test_declareMissingType(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.declare('/aninteger')
    
    
    # declare extra args.
    
    def test_declareTooManyArgs(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.declare('/aninteger', 'integer', '12345', (1,2,3))
    
    
    # set ok
    def test_setOk(self):
        self._api.declare('/aninteger', 'integer')
        self._api.set('/aninteger', '1234')
        var = nscldaq.vardb.variable.Variable(self._db, path='/aninteger')
        self.assertEquals('1234', var.get())
    
    # set abs
    
    def test_setAbspath(self):
        self._api.mkdir('/level1/level2')
        self._api.declare('/level1/level2/anint', 'integer')
        self._api.mkdir('/testing')
        self._api.cd('/testing')
        self._api.set('/level1/level2/anint', '1234')
        
        var = nscldaq.vardb.variable.Variable(self._db, path='/level1/level2/anint')
        self.assertEquals('1234', var.get())
        
    # set rel
    
    def test_setRelpath(self):
        self._api.mkdir('/level1/level2')
        self._api.declare('/level1/level2/anint', 'integer')
        self._api.mkdir('/testing')
        self._api.cd('/testing')
        self._api.set('../level1/level2/anint', '1234')
        
        var = nscldaq.vardb.variable.Variable(self._db, path='/level1/level2/anint')
        self.assertEquals('1234', var.get())
        
    # set badpath
    
    def test_setBadPath(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.set('/anint', '1234')
    
    # set invalid value.
    
    def test_setInvalidValue(self):
        self._api.declare('/anint', 'integer')
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.set('/anint', '3.1416')
    
    # get Ok
    
    def test_getOk(self):
        self._api.declare('/anint', 'integer', '1234')
        self.assertEquals('1234', self._api.get('/anint'))
    
    # get Abs
    
    def test_getAbsPath(self):
        self._api.mkdir('/this/that')
        self._api.declare('/this/that/theother', 'integer', '1234')
        self._api.cd('/this')
        
        self.assertEquals('1234', self._api.get('/this/that/theother'))
        
    # get Rel
    
    def test_getRelPath(self):
        self._api.mkdir('/this/that')
        self._api.declare('/this/that/theother', 'integer', '1234')
        self._api.mkdir('/test')
        self._api.cd('/test')
        
        self.assertEquals('1234', self._api.get('../this/that/theother'))
        
    # get Nosuch

    def test_getNoSuch(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.get('/testing')
            
    # defenum -ok
    
    def test_defenumOk(self):
        self._api.defineEnum('colors', ('red', 'green', 'blue'))
        self._api.declare('/acolor', 'colors')
        self.assertEquals('red', self._api.get('/acolor'))
    
    
    # defenum -dupValue.
    
    def test_defenumDupValue(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.defineEnum('colors', ('red', 'green', 'red'))
            
    # defenum -duptype
    
    def test_defenumDupType(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.defineEnum('integer', ('red', 'green', 'blue'))
    
    # defenum -badvlist
    
    def test_defenumBadValueList(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.defineEnum('integer', 'red')
    
    # defenum -badvalue (a value is not a string).
    
    def test_defenumBadValue(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.defineEnum('integer', ('red', 'green', ('blue', 'green')))
    
    
    # defsm ok
    
    def test_defsm_ok(self):
        self._api.defineStateMachine('simple', {'0': ('1', '0'), '1' : ('0',)})
        self._api.declare('test', 'simple')
        self.assertEquals('0', self._api.get('/test'))
    
    # defsm unreachable
    
    def test_defsm_unreachable(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.defineStateMachine(
                'unreachable', {'0': ('0', '1'), '1': ('0',), '2': ('1', '0')})
            
    # defsm badtransition
    
    def test_defsm_badTransition(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.defineStateMachine(
                'invalid', {'0':('0', '1')}
            )                                    # No state 1
    # defsm bad name
    
    def test_defsm_badTypeName(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.defineStateMachine('string', {'0': ('1', '0'), '1' : ('0',)})
            
    # defsm bad transitionlist
    
    def test_defsm_badTransitionList(self):
         with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.defineStateMachine('string', {'0': ('1', ('0', '1')), '1' : ('0',)})
    
    # defsm bad stateName
    
    def test_defsm_badStateName(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.defineStateMachine('string', {('0', '1'): ('1', ('0','1')), '1' : (('0', '1'),)})
            


    # ls default
    
    def test_ls_default(self):
        self._api.mkdir('/dir1')
        self._api.mkdir('/dir2')
        self._api.mkdir('/dir3')
        
        dirs = self._api.ls()
        self.assertEquals(3, len(dirs))
        dirs = sorted(dirs)
        self.assertEquals('dir1', dirs[0])
        self.assertEquals('dir2', dirs[1])
        self.assertEquals('dir3', dirs[2])
        
    # ls abspath
    
    def test_ls_abspath(self):
        self._api.mkdir('/dir1')
        self._api.mkdir('/dir2')
        self._api.mkdir('/dir3/subdir')
    
        dirs = self._api.ls('/')
        self.assertEquals(3, len(dirs))
        dirs = sorted(dirs)
        self.assertEquals('dir1', dirs[0])
        self.assertEquals('dir2', dirs[1])
        self.assertEquals('dir3', dirs[2])
    
    # ls relpath
    
    def test_ls_relpath(self):
        self._api.mkdir('/dir1')
        self._api.mkdir('/dir2')
        self._api.mkdir('/dir3/subdir')
        
        self._api.cd('/dir3/subdir')
        dirs = self._api.ls('../..')
        self.assertEquals(3, len(dirs))
        dirs = sorted(dirs)
        self.assertEquals('dir1', dirs[0])
        self.assertEquals('dir2', dirs[1])
        self.assertEquals('dir3', dirs[2])
        
    # ls noxpath
    
    def test_ls_noxpath(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.ls('/test')
    
    # lsvar_default
    
    def test_lsvar_default(self):
        self._api.declare('/anint', 'integer', '1234')
        self._api.declare('/areal', 'real', '3.1416')
        self._api.declare('/astring', 'string', 'hello world')
        
        vars = self._api.lsvar()
        self.assertEquals(3, len(vars))
        vars = sorted(vars, key=lambda x: x['name'])
        self.assertDictEqual(
            {'name':'anint', 'type':'integer', 'value':'1234'}, vars[0]
        )
        self.assertDictEqual(
            {'name':'areal', 'type':'real', 'value':'3.1416'}, vars[1]
        )
        self.assertDictEqual(
            {'name': 'astring', 'type' : 'string', 'value' : 'hello world'},
            vars[2]
        )
    
    # lsvar_abspath
    
    def test_lsvar_abspath(self):
        self._api.declare('/anint', 'integer', '1234')
        self._api.declare('/areal', 'real', '3.1416')
        self._api.declare('/astring', 'string', 'hello world')
        
        self._api.mkdir('/testdir')
        self._api.cd('/testdir')
        
        vars = self._api.lsvar('/')
        self.assertEquals(3, len(vars))
        vars = sorted(vars, key=lambda x: x['name'])
        self.assertDictEqual(
            {'name':'anint', 'type':'integer', 'value':'1234'}, vars[0]
        )
        self.assertDictEqual(
            {'name':'areal', 'type':'real', 'value':'3.1416'}, vars[1]
        )
        self.assertDictEqual(
            {'name': 'astring', 'type' : 'string', 'value' : 'hello world'},
            vars[2]
        )
    
    # lsvar_relpath
    
    def test_lsvar_relpath(self):
        self._api.declare('/anint', 'integer', '1234')
        self._api.declare('/areal', 'real', '3.1416')
        self._api.declare('/astring', 'string', 'hello world')
        
        self._api.mkdir('/testdir')
        self._api.cd('/testdir')
        
        vars = self._api.lsvar('..')
        self.assertEquals(3, len(vars))
        vars = sorted(vars, key=lambda x: x['name'])
        self.assertDictEqual(
            {'name':'anint', 'type':'integer', 'value':'1234'}, vars[0]
        )
        self.assertDictEqual(
            {'name':'areal', 'type':'real', 'value':'3.1416'}, vars[1]
        )
        self.assertDictEqual(
            {'name': 'astring', 'type' : 'string', 'value' : 'hello world'},
            vars[2]
        )
    
    
    # lsvar_noxpath
    
    def test_lsvar_noxpath(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.lsvar('/test')
            
            
    # rmvar ok
    
    def test_rmvarOk(self):
        self._api.declare('/anint', 'integer', '1234')
        self._api.declare('/pi', 'real', '3.14159')
        
        self._api.rmvar('/anint')
        
        vars = self._api.lsvar('/')
        self.assertEquals(1, len(vars))
        self.assertDictEqual(
            {'name': 'pi', 'type': 'real', 'value': '3.14159'}, vars[0]
        )
    
    # rmvar relpath
    def test_rmvarRelpath(self):
        self._api.declare('/anint', 'integer', '1234')
        self._api.mkdir('/test')
        self._api.cd('/test')
        
        self._api.rmvar('../anint')
        vars = self._api.lsvar('/')
        self.assertEquals(0, len(vars))
        
    # rmvar abspath
    
    def test_rmvarAbsPath(self):
        self._api.declare('/anint', 'integer', '1234')
        self._api.mkdir('/test')
        self._api.cd('/test')
        
        self._api.rmvar('/anint')
        vars = self._api.lsvar('/')
        self.assertEquals(0, len(vars))
        
    # rmvar noxvar.
    
    def test_rmvarNoxvar(self):
        with self.assertRaises(nscldaq.vardb.varmgr.error):
            self._api.rmvar('/anint')
    
    
if __name__ == '__main__':
    unittest.main()