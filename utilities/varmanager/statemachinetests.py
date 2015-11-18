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
# @file   statemachinetests.py
# @brief  Tests for the statemachine data type bindings
# @author <fox@nscl.msu.edu>

import unittest
import tempfile
import os
import sqlite3


import nscldaq.vardb.statemachine
import nscldaq.vardb.vardb
import nscldaq.vardb.variable
import nscldaq.vardb.enum

transitions = { 'first': ('second',), 'second' : ('third',),
                'third': ('first', 'second')}

class StateMachineTests(unittest.TestCase):
    def setUp(self):
        myVarDb = tempfile.NamedTemporaryFile()
        myVarDbName = myVarDb.name
        myVarDb.close()                    # Unix specfic.
        nscldaq.vardb.vardb.create(myVarDbName)
        self._db = nscldaq.vardb.vardb.VarDb(myVarDbName)
        self._tempFilename = myVarDbName

    def tearDown(self):
        os.remove(self._tempFilename)

    # Tests for create:
    
    def test_create_ok(self):
        id = nscldaq.vardb.statemachine.create(
            self._db, "statemachine", transitions
        )
        # Is there a new type 'statemachine' and does it have the correct id?
        
        sqlConn = sqlite3.connect(self._tempFilename)
        sqlCurs = sqlConn.cursor()
        rows = 0
        for row in sqlCurs.execute("SELECT id FROM variable_types WHERE type_name='statemachine'"):
            self.assertEquals(id, row[0])
            rows = rows + 1
        self.assertEquals(1, rows)
        
        
    def test_create_needdb(self):
        self.assertRaises(
            nscldaq.vardb.statemachine.error, nscldaq.vardb.statemachine.create,
            (1, 'statemachine', transitions)
        )
    def test_create_needstatemap(self):
        self.assertRaises(
            nscldaq.vardb.statemachine.error, nscldaq.vardb.statemachine.create,
            (self._db, "statemachine", 1)
        )
    def test_create_canmakevar(self):
        nscldaq.vardb.statemachine.create(
            self._db, 'statemachine', transitions
        )
        nscldaq.vardb.variable.create(self._db, None, '/myvar', 'statemachine')
        var = nscldaq.vardb.variable.Variable(self._db, path = '/myvar')
        self.assertEquals('first', var.get())
      
    # Tests for isStateMachine:
    
    def test_isStatemachine_yes(self):
        typeId = nscldaq.vardb.statemachine.create(
            self._db, 'statemachine', transitions
        )
        self.assertTrue(nscldaq.vardb.statemachine.isStateMachine(
            self._db, typeId
        ))
    def test_isStatemachine_no(self):
        nscldaq.vardb.enum.create(
            self._db, 'notstatemachine', ('first', 'second', 'third')
        )
        typeId = nscldaq.vardb.enum.id(self._db, 'notstatemachine')
        self.assertFalse(nscldaq.vardb.statemachine.isStateMachine(
            self._db, typeId
        ))
    # Tests for validNextStates
    
    def test_validnextstates_type_first(self):
        typeId = nscldaq.vardb.statemachine.create(
            self._db, 'statemachine', transitions
        )
        nextStates = nscldaq.vardb.statemachine.validNextStates(
            self._db, typeId, 'first'
        )
        self.assertEquals(1, len(nextStates))
        self.assertEquals('second', nextStates[0])
        
    def test_validnextstates_type_third(self):
        typeId = nscldaq.vardb.statemachine.create(
            self._db, 'statemachine', transitions
        )
        nextStates = nscldaq.vardb.statemachine.validNextStates(
            self._db, typeId, 'third'
        )
        self.assertEquals(2, len(nextStates))
        self.assertEquals('first', nextStates[0])
        self.assertEquals('second', nextStates[1])
        
    def test_validnextstates_var_first(self):
        typeId = nscldaq.vardb.statemachine.create(
            self._db, 'statemachine', transitions
        )
        nscldaq.vardb.variable.create(self._db, None, '/myvar', 'statemachine')
        var = nscldaq.vardb.variable.Variable(self._db, path='/myvar')
        nextStates = nscldaq.vardb.statemachine.validNextStates(
            self._db, var.getId()
        )
        self.assertEquals(1, len(nextStates))
        self.assertEquals('second', nextStates[0])
        
    def test_validnextstates_var_third(self):
        typeId = nscldaq.vardb.statemachine.create(
            self._db, 'statemachine', transitions
        )
        nscldaq.vardb.variable.create(self._db, None, '/myvar', 'statemachine')
        var = nscldaq.vardb.variable.Variable(self._db, path='/myvar')
        var.set('second')
        var.set('third')
        nextStates = nscldaq.vardb.statemachine.validNextStates(
            self._db, var.getId()
        )
        self.assertEquals(2, len(nextStates))
        self.assertEquals('first', nextStates[0])
        self.assertEquals('second', nextStates[1])
        
    def test_validnextstates_wrongparamcount(self):
        typeId = nscldaq.vardb.statemachine.create(
            self._db, 'statemachine', transitions
        )
        self.assertRaises(
            nscldaq.vardb.statemachine.error,
            nscldaq.vardb.statemachine.validNextStates,
            (self._db, typeId, 'first', 'extra')
        )
    
    # tests for getTransitionMap
    
    def test_gettransition_map(self):
        typeId = typeId = nscldaq.vardb.statemachine.create(
            self._db, 'statemachine', transitions
        )
        transitionMap = nscldaq.vardb.statemachine.getTransitionMap(
            self._db, typeId
        )
        self.assertEquals(transitions, transitionMap)
   

if __name__ == '__main__':
    unittest.main()
    




