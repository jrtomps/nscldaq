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
# @file   project.py
# @brief  Mediates access to the project database.
# @author <fox@nscl.msu.edu>
import sqlite3


#-----------------------------------------------------------------------------
##
# @class Project
#
#   This class is responsible for schema creation/test.
#
class Project:
    def __init__(self, dbPath):
        self.connection = sqlite3.connect(dbPath)
        self.connection.execute('PRAGMA foreign_keys = on')
    ##
    # open
    #   Open a database that's already been initialized/created.
    #   This will check the existence of the appropriate set of
    #   tables.
    #
    def open(self):
        curs = self.connection.cursor()
        curs.execute(
            '''
            SELECT COUNT(*) FROM sqlite_master WHERE type='table' and name='hosts'
            '''
        )
        if (1,) != curs.fetchone():
            raise RuntimeError('Project.open - missing hosts table')
    
    ##
    # create
    #   Create a new project in the dbPath
    #
    def create(self):
        self.connection.execute(
            '''CREATE TABLE hosts (
                id           INTEGER PRIMARY KEY,
                host_name    VARCHAR(64) NOT NULL
            )'''
        )
        self.connection.execute(
            '''CREATE TABLE rings (
                id        INTEGER PRIMARY KEY,
                name      VARCHAR(128) NOT NULL,
                host_id   INTEGER NOT NULL,
                sourceid  INTEGER DEFAULT NULL,
                FOREIGN KEY (host_id) REFERENCES hosts(id)
            )
            '''
        )
        self.connection.execute(
            '''CREATE TABLE programs (
                id           INTEGER PRIMARY KEY,
                path         VARCHAR(128) NOT NULL,
                working_dir  VARCHAR(128) NOT NULL,
                host_id      INTEGER NOT NULL,
                FOREIGN KEY (host_id) REFERENCES hosts(id)
            )
            '''
        )
        self.connection.execute(
            '''CREATE TABLE program_args (
                 id        INTEGER PRIMARY KEY,
                 argument  VARCHAR(128) NOT NULL,
                 program_id INTEGER NOT NULL,
                 FOREIGN KEY (program_id) REFERENCES programs(id)
            )
            '''
            )
        self.connection.execute(
            '''CREATE TABLE data_sources (
                id          INTEGER PRIMARY KEY,
                program_id  INTEGER NOT NULL,
                ring_id     INTEGER NOT NULL,
                FOREIGN KEY (program_id) REFERENCES programs(id),
                FOREIGN KEY (ring_id)    REFERENCES rings(id)
            )
            '''
        )
        
        self.connection.execute(
            '''
            CREATE TABLE consumers (
                id          INTEGER PRIMARY KEY,
                program_id  INTEGER NOT NULL,
                ring_id     INTEGER NOT NULL,
                FOREIGN KEY (program_id) REFERENCES programs(id),
                FOREIGN KEY (ring_id)    REFERENCES rings(id)
            )
            '''
        )
 
        self.connection.execute(
            '''
            CREATE TABLE loggers (
                id              INTEGER PRIMARY KEY,
                ring_id         INTEGER NOT NULL,
                host_id         INTEGER NOT NULL,
                program_id      INTEGER DEFAULT NULL, 
                log_dir         VARCHAR(128),
                FOREIGN KEY (program_id) REFERENCES programs(id),
                FOREIGN KEY (ring_id)    REFERENCES rings(id),
                FOREIGN KEY (host_id)    REFERENCES hosts(id)
            )
            '''
        )

        self.connection.execute(
            '''
            CREATE TABLE services (
                id          INTEGER PRIMARY KEY,
                port_no     INTEGER DEFAULT NULL,
                managed     INTEGER,
                service_name VARCHAR(32) DEFAULT NULL
            )
            '''
        )

        self.connection.execute(
            ''' 
            CREATE TABLE eventbuilder (
                id             INTEGER PRIMARY KEY,
                ring_id        INTEGER NOT NULL,
                host_id        INTEGER NOT NULL,
                port           INTEGER DEFAULT NULL,
                service_id     INTEGER NOT NULL,
                FOREIGN KEY (ring_id) REFERENCES rings(id),
                FOREIGN KEY (host_id) REFERENCES hosts(id),
                FOREIGN KEY (service_id) REFERENCES services(id)
            )
            '''
        )
        self.connection.execute(
            '''
            CREATE TABLE evb_datasources (
                id              INTEGER PRIMARY KEY,
                ring_id         INTEGER NOT NULL,
                evb_id          INTEGER NOT NULL,
                host_id         INTEGER NOT NULL,
                ts_extractor    VARCHAR(128) NOT NULL,
                FOREIGN KEY (ring_id) REFERENCES rings(id),
                FOREIGN KEY (evb_id)  REFERENCES eventbuilder(id),
                FOREIGN KEY (host_id) REFERENCES hosts(id)
            )
            '''
        )
        self.connection.execute(
            ''' -- There can be only one.
            CREATE TABLE statemanager (
                id       INTEGER PRIMARY KEY,
                host_id  INTEGER NOT NULL,
                service_id INTEGER NOT NULL,
                FOREIGN KEY (host_id) REFERENCES hosts(id),
                FOREIGN KEY (service_id) REFERENCES services(id)
            )
            '''
        )
        self.connection.execute('''
            CREATE TABLE state_aware_programs (
                id              INTEGER PRIMARY KEY,
                statemgr_id     INTEGER,
                FOREIGN KEY (statemgr_id) REFERENCES statemanager(id)
            )
            '''
        )
        
        
##
# @class Hosts
#
#   This class is responsible for maintaining the hosts known to the project.
#   The hosts are used to stock various drop down lists that select the host
#   on which objects (ring, programs) will be instantiated.
#
#  See the Project class.
class Hosts:
    def __init__(self, project):
        self._project = project
        
    ##
    # _exists
    #  Returns true if a host is already in the table.
    #
    # @param host - name of the host.
    # @param column - The column to match with
    #
    def _exists(self, host, column='host_name'):
        conn = self._project.connection
        curs = conn.cursor()
        query = '''
            SELECT COUNT(*) FROM hosts WHERE %s = ?
            ''' % (column,)
        curs.execute(
            query, (host,)
        )
        return (0,) != curs.fetchone()
        
        

        
    # Client accessible methods
    
    ##
    # add
    #   Add a new host to the system.
    #   It is a RuntimeError to add a host that's already known.
    #   note that caller should probably take all hosts and convert them inot
    #   FQDN's so that charlie  == charlie.nscl.msu.edu
    #
    # @param host - host to add.
    # @return id (primary key of the host added)
    #
    def add(self,host):
        conn = self._project.connection
        curs = conn.cursor()
        if self._exists(host):
            raise RuntimeError("Project.Hosts.add - duplicate host")
        curs.execute(
            '''
            INSERT INTO hosts (host_name) VALUES (?)
            ''', (host,)
        )
        return curs.lastrowid
        
    ##
    #  list
    #    produces a list of the hosts in the table.  The hosts are
    #    sorted by host name.
    #
    # @return list of dicts.  Each dict  has the keys
    #          *  id        - Primary key of the record.
    #          *  host_name -  Name of the host.
    #
    def list(self):
        result = list()
        
        # Setup so that the query will use the row factory to give us key/value
        # pairs on field names.
        
        conn = self._project.connection
        oldFactory = conn.row_factory
        conn.row_factory = sqlite3.Row
        curs = conn.cursor()
         
        curs.execute('''
            SELECT * FROM hosts ORDER BY host_name ASC
                     ''')
        
        # Fetch the records out in to the result.
        
        row = curs.fetchone()
        while row is not None:
            record = {'id': row['id'], 'host_name': row['host_name']}
            result.append(record)
            row = curs.fetchone()
            
        # Don't forget to restore the original row factory.
        
        conn.row_factory = oldFactory
        return result
    
    ##
    # exists
    #   Determins if a specific host is in the hosts table
    #
    # @param host - the host we are checking for.
    #
    # @return boolean - True if the host is in the table, false otherwise.
    #
    def exists(self,host):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT COUNT(*) FROM hosts WHERE host_name = ?
                    ''', (host,))
        return (curs.fetchone() != (0,))
    
    ##
    # id
    #   Returns the id of a host
    #
    # @param host - name of the host to lookup.
    # @return mixed
    # @retval integer - id of the host if it exists.
    # @retval None    - If the host does not exist.
    #
    def id(self, host):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT id FROM hosts WHERE host_name = ?
                     ''', (host,))
        row = curs.fetchone()
        if row is None:
            return None
        else:
            return row[0]
    
    ##
    # delete
    #   Deletes a row from the hosts table.
    #
    # @param id - The id (primary key) of the host to delete.
    #
    # @throw - It is RuntimeError to attempt to delete a nonexistent row.
    #
    def delete(self, id):
        conn = self._project.connection
        if self._exists(id, 'id'):
            conn.execute('''
                DELETE FROM hosts WHERE id=?                     
                         ''', (id,))
        else:
            raise RuntimeError('project.Hosts.delete - no such host to delete')
            
#------------------------------------------------------------------------------

##
# @class Rings
#
#   Manages the rings table ensuring that the host_id is a valid host
#
class Rings:
    def __init__(self, project):
        self._project = project
    
    
    def _exists(self, ringname, hostId):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT COUNT(*) FROM rings WHERE name = ? AND host_id = ?
                     ''', (ringname, hostId)
        )
        return ((0,) != curs.fetchone())
    ##
    # _existsId
    #   Return true if the specified ring id exists.
    #
    # @param id - the id of the ring to test.
    #
    def _existsId(self, id):
        conn = self._project.connection
        curs = conn.cursor()
        curs.execute('''
            SELECT COUNT(*) FROM rings WHERE id = ?
            ''', (id,))
        return ((0,) != curs.fetchone())
    ##
    # _query
    #   Common code to query everything about ring(s)
    # @param params - Parameterization of the query (stuff to fill in ?)
    # @param order - If supplied is an ORDER BY clause for the query.
    # @param where - If specivfied is a WHERE clause for the query.
    #
    def _query(self, params, order = '', where = ''):
        conn   = self._project.connection
        oldFact= conn.row_factory
        conn.row_factory = sqlite3.Row
        curs   = conn.cursor()
            
        query = '''
            SELECT r.id AS ring_id, r.name AS ring_name, r.sourceid,
                   h.id AS host_id, h.host_name AS host_name
                   FROM rings r
                   INNER JOIN hosts h ON h.id = r.host_id
                   %s
                   %s ''' % (where, order)
        curs.execute(query, params)
    
        result = list()
        for r in curs:
            result.append(r)
        
        conn.row_factory = oldFact
        
        return result
    #-------------------------------------------------------------------------
    # Public interface
    
    ##
    # add_withHostId
    #   Add a ring to the system given the host by id:
    #
    # @param name       - Name of the ring
    # @param hostid     - Id of the host the ring is created in
    # @param sourceid=None - If supplied, the source id associated with the
    #                     ring producer.
    # @return int - The id of the element added to the Rings table.
    # @throw RuntimeError - If the host id does not exist.
    # @throw RuntimeError - The ring already exists on that host.
    #
    def add_withHostId(self, name, hostid, sourceid=None):
        
        # Throw a runtime error if the hostid is not valid:
        
        hosts = Hosts(self._project)
        if not hosts._exists(hostid, 'id'):
            raise RuntimeError('project.Rings.add_withHostId - No such host')
            
        # Throw a runtime error if the ring/hostid combination already exist.
        
        if self._exists(name, hostid):
            raise RuntimeError('project.Rings.add_withHostid - This ring already is defined on that host')
        
        conn   = self._project.connection
        curs   = conn.cursor()
        
        curs.execute('''
            INSERT INTO rings (name, host_id, sourceid) VALUES(?,?,?)
                     ''', (name, hostid, sourceid)
        )
        return curs.lastrowid
    
    ##
    # add_withHostname
    #
    #  Add a ring to the system given the host by hostname.
    #
    # @param name          - Name of the ring.
    # @param hostname      - Name of the host the ring is created in.
    # @param sourceid=None - If provided the id source id associated with
    #                        the ring producer.
    # @param create=False  - If true and the hostname is not in the
    #                        hosts table it is created.
    # @return int   - The id of the element added to the rings table.
    # @throw RuntimeError - if create is False and the hostname is not
    #                       in the hosts table.
    # @throw RuntimeError - if the ring already has been defined on that host.
    #
    def add_withHostname(self, name, hostname, create=False, sourceid=None):
        hosts = Hosts(self._project)
        id    = hosts.id(hostname)
        if (id is None) and create:
            id = hosts.add(hostname)
        return self.add_withHostId(name, id, sourceid)
    
    ##
    # list
    #   Provides a list of the rings known to the system.
    #
    # @return list of dicts  Each dict has the following keys:
    #         * ring_id   - Primary key of the ring in rings table.
    #         * ring_name - Name of the ring.
    #         * sourceid  - Data source id (might be empty).
    #         * host_id   - Id of the host the ring is in.
    #         * host_name - Name of the host the ring is in.
    # The list is ordered by ring name, subordered by host name.
    #
    #
    def list(self):        
        return self._query((), 'ORDER BY ring_name, host_name ASC')    
    
    ##
    # find
    #   Searches for a specific ring and returns its information
    #   Note multiple matches are possible unless the hostname is
    #   provided.
    #
    #  @param ringName  - Name of the ring to find.
    #  @param hostName (None) If supplied the search is qualified by the hostName.
    #
    # @return list of dicts - see list above for the keys in the dict.
    #
    def find(self, ringName, hostName=None):
        if hostName is None:
            whereClause = 'WHERE ring_name=?'
            bindings    = (ringName,)
        else:
            whereClause = 'WHERE ring_name=? AND host_name = ?'
            bindings    = (ringName, hostName)
        
        return self._query(
            bindings, 'ORDER BY ring_name, host_name ASC',
            whereClause
        )

    ##
    # exists
    #   tests for existence of a ring
    #
    # @param ringName - Name of the ring to check for.
    # @param hostName - Name of the host the ring lives in.
    #
    # @return boolean
    #
    def exists(self, ringName, hostName):
        list = self.find(ringName, hostName)
        return len(list) != 0
    ##
    # delete
    #   Deletes a specific ring
    #
    #  @param id - Id of the ring to delete.
    #  @throw RuntimeError if the ring does not exist.
    #
    def delete(self, id):
        
        conn = self._project.connection
        curs = conn.cursor()
        
        curs.execute('''
            SELECT COUNT(*) FROM rings WHERE id = ?
                     ''', (id,))
        if curs.fetchone() == (0,):
            raise RuntimeError('project.Rings.delete - no such ring.')
        
        conn.execute('''
            DELETE FROM rings WHERE id = ?
                     ''', (id,))
    ##
    # delete_byname
    #
    #   Deletes a ring by name
    #
    # @param name - ring name.
    # @param host - host the ring lives in.
    #
    # @throw RuntimeError if the ring does not exist.
    #
    def delete_byname(self, name, host):
        items = self.find(name, host)
        if len(items) == 0:
            raise RuntimeError('project.Rings.delete_byname - no such ring')
        id = items[0]['ring_id']
        self.delete(id)
        
    ##
    # modify
    #   Modify a ring's properties.
    #
    #    @param id - The id of the ring to modify.
    #    @param **attributes  - Key/values of the things that change which can
    #                           be any of:
    #                      *   name     - change the name of the ring.
    #                      *  host_name - Change the name of the host (must exist).
    #                      *  sourceid  - Change the value of the sourceid.
    #   @throw RuntimeError - id is not an existing ring.
    #   @throw RuntimeError - host_name is provided but is not an existing host.
    #   @throw RuntimeError - After host_name has been applied if needed, the
    #                         resulting ring/host would duplicate another ring/host.
    #   @throw RuntimeError - A key is provided that is not legal.
    #
    def modify(self, id, **attributes):
        host = Hosts(self._project)
        
        # raise error if the ring does not exist.
        
        old = self._query((id,), '', 'WHERE ring_id=?')
        if len(old) == 0:
            raise RuntimeError('project.Rings.modify - no such ring')
        old = old[0]
        
        # raise error if there are bad keys:
        
        validKeys = set(['name', 'host_name', 'sourceid'])
        if len(set(attributes.keys()) - validKeys) > 0:
            raise RuntimeError('project.Rings.modify - invalid attribute key(s)')
            
        fields = list()
        values = list()
        for field in attributes.keys():
            #
            #  host_name -> host_id:
            if field == 'host_name':
                host = Hosts(self._project)
                hostid = host.id(attributes[field])
                # Raise an error if the host id is None:
                
                if hostid is None:
                    raise RuntimeError('project.Rings.modify - no such new host')
                
                fields.append('='.join(('host_id', '?')))
                values.append(hostid)
            else :
                # All others are name-> name.
                values.append(attributes[field])
                fields.append('='.join((field, '?')))
        
        values.append(id)
            
        # Check for and throw if the final host/ring name are duplicates:
        
        changing = False
        newHost = old['host_name']
        newRing = old['ring_name']
        if 'host_name' in attributes.keys():
            newHost = attributes['host_name']
            changing = True
        if 'name' in attributes.keys():
            newRing = attributes['name']
            changing = True
        
        # If the host/ring don't change we're ok otherwise need to check.
        
         
        if changing and self.exists(newRing, newHost):
            raise RuntimeError('project.Rings.modify - would make a duplicate ring')
        
        #  Combine fields into a set clause and construct the query:
        
        setFields = ', '.join(fields)
        query = '''
            UPDATE rings SET %s WHERE id = ?
        ''' % setFields
        
        self._project.connection.execute(query, values)
        
    ##
    # modify_byname
    #   Modify a ring's properties given its name
    #
    #  @param name  - Name of the ring to modify.
    #  @param host  - Host the ring is on.
    #  @param attributes - see modify() above.
    #
    #  @note this method figures out the id for name, host and then
    #        invokes modify..
    #
    def modify_byname(self, name, host, **attributes):
        ring = self.find(name, host)
        if len(ring) == 0:
            raise RuntimeError('project.Rings.modify -- no such ring')
        else:
            modify(ring[0]['ring_id'], **attributes)
    
   #---------------------------------------------------------------------------
   #
   
   
##
# @class Programs
#   This class manages the programs and program_args tables which together
#   define a program and its command line parameters.

class Programs:
    def __init__(self, project):
           self._project = project
           self._hosts   = Hosts(self._project)

    ##
    # _getArgs
    #   Get the arguments associated with a program.
    #
    # @param id - The id of the program.
    # @return list (posibly empty)
    #
    def _getArgs(self, id):
        conn = self._project.connection
        curs = conn.cursor()
        
        curs.execute('''
            SELECT argument FROM program_args WHERE program_id = ?
                     ''', (id,))
        result = list()
        for r in curs:
            result.append(r[0])
            
        return result

    ##
    # _query
    #   Perform a query that joins to the args table and marshalls stuff
    #   into the sort of dict returned by list or find.
    #
    # @param where  - If present a WHERE clause for the query.
    # @param qargs   -  query arguments (plug into where:)
    # @return dict - see list
    #
    def _query(self, where, qargs):
        conn       = self._project.connection
        oldFactory = conn.row_factory
        conn.row_factory = sqlite3.Row
        cursor     = conn.cursor()
        
        query = '''
            SELECT p.id AS program_id, p.path, p.working_dir,
                    h.id AS host_id, h.host_name
                FROM programs p
                INNER JOIN hosts h ON h.id = p.host_id
                       ''' 
        query += where
        cursor.execute(query, qargs)
        result = list()
        for row in cursor:
            args = self._getArgs(row['program_id'])
            r = dict(row)
            r['args'] = args
            result.append(r)
        
        conn.row_factory = oldFactory
        return result

    ##
    # _replaceArgs
    #
    #   Replace the arguments for a program (the caller is responsible
    #   for managing the atomicity of this)
    #
    #  @param conn - Connection
    #  @param id   - Id of the program.
    #  @param newargs - List of new paramters.
    # 
    def _replaceArgs(self, conn, id, newargs):
        #
        #  Kill off the old args:
        #
        conn.execute('''
            DELETE FROM program_args WHERE program_id = ?
                     ''', (id,))
        #
        # Crezte the new ones:
        for arg in newargs:
            conn.execute('''
                INSERT INTO program_args (program_id, argument) VALUES (?,?)
                        ''', (id, arg))
            

    #--------------------------------------------------------------------------
    #  Public interface
    
    ##
    # add
    #   Add a new program.
    #
    # @param path        - path to the program executable.
    # @param workingdir  - Directory the program will run in.
    # @param host_id     - Id  of host on which the program runs (hosts table).
    # @param args        - List of additional command line parameters.
    #
    # @return integer    - The id of the program (primary key in programs table).
    # @throw RuntimeError - If host_id does not exist.
    #
    def add(self, path, workingdir, host_id, args=list()):
        
        # Ensure the host exists:
        
        if not self._hosts._exists(host_id, 'id'):
            raise RuntimeError('projects.Programs.add - no such host.')
        
        conn = self._project.connection
        curs = conn.cursor()
        
        # Do the inserts as a single transaction
        
        with conn:
            curs.execute('''
                INSERT INTO programs (path, working_dir, host_id)
                    VALUES(?,?,?)
                         ''', (path, workingdir, host_id))
            id = curs.lastrowid
            for arg in args:
                curs.execute('''
                    INSERT INTO program_args (argument, program_id)
                        VALUES (?,?)
                             ''', (arg, id))
        return id
        
    ##
    # add_byHostname
    #    For the most part this just
    #    looks up the host and calls add
    #
    # @param path         - Path to the program.
    # @param workingdir   - Working directory for the program.
    # @param host_name    - Name of the host on which the program run.s
    # @param args         - Additional program parameters.
    # @param create       - If truem, create the host if it does not exist.
    #
    # @return integer - id of the newly created program record.
    # @throw RuntimeError - the host_name is not a defined host and create =False
    def add_byHostname(self, path, workingdir, host_name, args=list(), create=False):
        hostid = self._hosts.id(host_name)
        if (hostid is None) and create:
            hostid = self._hosts.add(host_name)
        return self.add(path, workingdir, hostid, args)
        
    ##
    # list
    #   Lists all programs and their associated arguments.
    #
    # @return list of dicts   each dict has the following keys:
    #      -  program_id  - Id of the program (primary key)
    #      -  path        - Program path
    #      -  working_dir - Working directory for the program.
    #      -  host_id     - Id of the target host.
    #      -  host_name   - Name of the target host.
    #      -  args        - possibly empty argument list.
    #
    #  Order is by the program_id which is  the order in which the
    #  records where added
    #
    def list(self):
        return self._query("", list())
    ##
    # find
    #   A generic find for programs.
    #
    # @param terms - The values of this parameter represent strings that
    #                will turn into WHERE xxx LIKE value
    #                that will all be anded together to filter the
    #                query.  Possible keys are
    #      -  program_id  - Id of the program (primary key)
    #      -  path        - Program path
    #      -  working_dir - Working directory for the program.
    #      -  host_id     - Id of the target host.
    #      -  host_name   - Name of the target host.
    #
    # For example
    #   {'path': /user/fox/programs/%', host_name: 'spdaq22.nscl.msu.edu'}
    # as the value for terms limits the result set to those programs that
    # are in the /user/fox/programs/ directory that will be started on
    # spdaq22.nscl.msu.edu
    #
    # @return - see list
    # @throw  - If one of the keywords is not a legitimate field selector.
    # @note   - Invalid host names or ids, or program ids are not errors, they
    #           just will return a null result set.
    # @throw RuntimeError - if there's a bad keyword.
    #
    def find(self, **terms):
        # Ensure all the keywords are good, otherwise throw a RuntimeError
        
        validKeys = set(['program_id', 'path', 'working_dir', 'host_id', 'host_name'])
        usedKeys  = set(terms.keys())
        badKeys = usedKeys - validKeys
        if len(badKeys) != 0:
            raise RuntimeError(
                'project.Programs.find - One or more invalid find keys wer provided %r' % badKeys
        )
        # Build and perform the query.
        where = ""
        wheres = list()
        values = list()
        for key in terms.keys():
            value = terms[key]
            wheres.append('%s LIKE ?' % key)
            values.append(value)
        if len(wheres) > 0:
            where = 'WHERE '
            where += 'AND '.join(wheres)
        return self._query(where, values)
    
    
    ##
    # ids
    #   Returns the program ids for a set of programs that match some
    #   criteria.
    # @param **terms - See find above for the meaning of those terms.
    #
    # @return list - possibly empty, of ids (primary keys) of programs that match
    # @throw see find.
    def ids(self, **terms):
        listing = self.find(**terms)
        result = list()
        for row in listing:
            result.append(row['program_id'])
        return result
    
    ##
    # modify
    #   Modify a program's attributes.
    #
    # @param id - the program id to modify.
    # @param **attributes - key/newvalue for the following keys:
    #      -  path        - Program path
    #      -  working_dir - Working directory for the program.
    #      -  host_id     - Id of the target host.
    #      -  host_name   - Name of the target host.
    #      -  args        - Parameters of the program.
    #
    # @throw - if host_id and host_name are both supplied (even if consistent).
    # @throw - if host_id, or host_name does not match an existing host.
    # @throw - if an invalid key is provided in the attributes.
    # @throw - If an invalid id is specified.
    #
    def modify(self, id, **attributes):
        baseAttributes   = set(['path', 'working_dir'])
        hostAttributes = set(['host_name', 'host_id'])
        otherAttributes = set(['args'])
        
        #  Ensure the attributes are all legal:
        
        allowedAttributes = baseAttributes | hostAttributes | otherAttributes
        if len(set(attributes.keys()) - allowedAttributes) > 0:
            raise RuntimeError(
                'project.Programs.modify -- Invalid attributes specified'
            )
            
        # Not allowed to have host_id and host_name:
        
        
        if len(hostAttributes & set(attributes.keys())) > 1 :
            raise RuntimeError(
                'project.Programs.modify - Cannot specify both host_id and host_name'
            )
        
        # The id must exist:
        
        if len(self.ids(program_id=id)) == 0:
            raise RuntimeError(
                'project.Programs.modify - No such program id'
            )
        
        # This is a transaction so it's all or nothing, no partial changes.
        
        conn = self._project.connection
        with conn:
                
            baseAttributes   = set(['path', 'working_dir'])
            
            #
            # Base attributes:
            #
            fields = list()
            values = list()
            for key in baseAttributes:
                if key in attributes.keys():
                    fields.append('%s=?' % key)
                    values.append(attributes[key])
                    
            # If there fields we need to do the update, generate/execute
            # the update statement:
            
            if len(fields) > 0:
                values.append(id)         # For the WHERE field.     
                sql = 'UPDATE programs SET %s WHERE id = ?' % ','.join(fields)
                conn.execute(sql, values)
            
            # host_id requires a bit of checking to be sure it's legal.
            # and to be sure host_name isn't present too.
            
            host_id = None
            if 'host_id' in attributes.keys():
                host_id = attributes['host_id']
                if not self._hosts._exists(host_id, 'id'):
                    raise RuntimeError(
                        'project.Programs.modify - host_id does not exist'
                    )
                    
            # host_name requires a lookup and then modify the host id:
           
            if 'host_name' in attributes.keys():
                host_id = self._hosts.id(attributes['host_name'])
                if host_id is None:
                    raise RuntimeError(
                        'project.Programs.modify - host_name does not exist'
                    )                
                
            if host_id is not None:
                sql = 'UPDATE programs SET host_id = ? WHERE id = ?'
                conn.execute(sql, (host_id, id))
                
            # Args:
            
            if 'args' in attributes.keys():
                newargs = attributes['args']
                self._replaceArgs(conn, id, newargs)
    
    ##
    # Delete a program given its id.
    #
    # @param id - the id of the program.
    #
    # @throw  if id is not a valid program id.
    #
    def delete(self, id):
        
        # Ensure the program exists:
        
        if len(self.ids(program_id=id)) == 0:
            raise RuntimeError(
                'project.Programs.delete - no such program id'
            )
        
        # Do this and the args in a transaction:
        
        conn = self._project.connection
        with conn:
            # Replace the parameters with an empty list to delete them:
            
            self._replaceArgs(conn, id, list())
            conn.execute('''
                DELETE FROM programs WHERE id = ?
                         ''', (id,)
            )

#----------------------------------------------------------------------------


##
# @class DataSource
#
#   A data source is a program that is a producer for a ring.
#   A data source program is constrained to live in the same host
#   on which its ring is defined.  That is an NSCLSDAQ restriction.
#   it is implied that a data source will have a feeder to the event builder
#   if the event builder is defined.
#   Sources have user choosable source source_id values but these must be
#   unique across the proejct.
#
class DataSource:
    def __init__(self, project):
        self._project = project
        self._rings   = Rings(project)
        self._hosts   = Hosts(project)
        self._programs= Programs(project)
        
    ##
    #  Add a new data source.
    #
    # @param sourceId - unique source Id.
    # @param program  - Id of the program that will source data to the ring.
    #
    # @return integer - The id of the added data source.
    #
    # @throw - ring does not exist.
    # @throw - Ring already has a data source.
    # @throw - Program does not exist.
    # @throw - Program is already a data source.
    # @throw - Program not in the same host as the ring.
    #
    def add(self, ring, program):
        
        # The ring must exist:
        
        if not self._rings._existsId(ring):
            raise RuntimeError(
                'project.DataSource.add - No such ring id %d' % (ring,)
            )
            
        # The program must exist:
        
        if len(self._programs.ids(program_id = program)) == 0:
            raise RuntimeError(
                'project.DataSource.add - No such program id %d' % (program, )
            )
        #  The ring must not already have a source:
        
        curs = self._project.connection.cursor()
        curs.execute('''
            SELECT COUNT(*) FROM data_sources WHERE ring_id = ?
            ''', (ring,)
        )
        count = curs.fetchone()
        if count[0] > 0:
            raise RuntimeError(
                'project.DataSource.add - Ring %d already has a source' % (ring,)
            )
        
        # The program can't already be a source.
        
        curs.execute('''
            SELECT ring_id FROM data_sources WHERE program_id = ?
            ''', (program,)
        )
        ringid = curs.fetchone()
        if ringid is not None:
            raise RuntimeError(
                'project.DataSource.add -  Program %d is already a source for %d' 
                   % (program, ringid[0])
            )
        
        # The program's host id must be the ring's host id.
        
        curs.execute('''
            select r.host_id AS rhost, p.host_id AS phost
                FROM hosts h
                INNER JOIN rings r    ON r.host_id = h.id
                INNER JOIN programs p ON p.host_id = h.id
                WHERE p.id = ?
                AND   r.id = ?
            ''', (program, ring))
        r = curs.fetchone()

        if r is None:
            raise RuntimeError(
                'project.DataSource.add - Program %d is not in the same host as ring %d'
                % (program, ring)
            )
        
        # Everything's sane so add the source to the table:
        
        curs.execute('''
           INSERT INTO data_sources (program_id, ring_id) VALUES (?,?)
        ''', (program, ring)
        )
        return curs.lastrowid
    
    ##
    # addWithName
    #
    #    Adds a data source to a ring given its ring name.
    #    -   Get the ring id from its name
    #    -   If there is no ring it raise a RuntimeError
    #    -   If ther is a ring, just invoke add.
    #
    # @param ringName - Name of the ring for which the program is the data source.
    # @param program  - id of the program.
    #
    # @return integer - The id of the source added.
    #
    # @throw - see add, however not that if our lookup of the ringName fails,
    #          we'll throw for that so we can provide a better message.
    #
    def addWithName(self, ringName, program):
        
        cursor = self._project.connection.cursor()
        cursor.execute('''
            SELECT  id FROM rings WHERE name=?
        ''', (ringName,))
        
        row = cursor.fetchone()
        
        if row is None:
            raise RuntimeError(
                'project.DataSource.addWithName - No such ring %s' % (ringName,)
            )
        
        self.add(row[0], program)

    ##
    # list
    #
    #   Provide a list of all of the data sources.
    #
    # @return list of dicts  where each dict has the following keys:
    #        *   id           - The id of the source (used to delete it).
    #        *   ring_info    - A dict containing the data from Ring.list
    #        *   program_info - A dict containing the data from Program.list
    #        data will be returned in id order.
    #
    def list(self):
        result = list()
        
        oldFactory = self._project.connection.row_factory
        self._project.connection.row_factory = sqlite3.Row
        cursor = self._project.connection.cursor()
        cursor.execute('''
            SELECT s.id AS id,
                   p.id AS program_id, p.path, p.working_dir,
                   h.id AS host_id, h.host_name,
                   r.id AS ring_id, r.name AS ring_name, r.sourceid
            FROM data_sources s
            INNER JOIN programs p ON p.id = s.program_id
            INNER JOIN hosts    h ON h.id = p.host_id
            INNER JOIN rings    r ON r.id = s.ring_id
        ''')
        
        r = cursor.fetchone()

        while r is not None:
            ringInfo = {
                'ring_id': r['ring_id'], 'ring_name': r['ring_name'],
                'sourceid': r['sourceid'], 'host_id': r['host_id'],
                'host_name': r['host_name']
            }
            args = self._programs._getArgs(r['program_id'])
            programInfo = {
                'program_id': r['program_id'], 'path': r['path'],
                'working_dir': r['working_dir'], 'host_id': r['host_id'],
                'host_name': r['host_name'], 'args': args
            }
            result.append({
                'id': r['id'], 'program_info': programInfo,
                'ring_info': ringInfo
            })
            
            r = cursor.fetchone()
            
        
        self._project.connection.row_factory = oldFactory
        return result

    ##
    # ringSource
    #    Returns information about the program that is the ring source
    #    for the named ring.
    #
    # @param ringname - Name of the ring for which the information is returned
    # @param host     - name of the host the ring is on.
    #
    # @return mixed
    # @retval none - There is no such ring or there is no source for that ring.
    # @retval dict - A dictionary with keys that are:
    #                *  ring_id      - Id of the ring.
    #                *  sourceid     - The source id.
    #                *  program_info - dict contanining the information about the
    #                                  source program.  The keys for this are
    #                                  described in Program.list
    #
    #
    def ringSource(self, ringname, host):
        
        # Determine the ring id:
        
        ring = self._rings.find(ringname, host)
        if len(ring) == 0:
            return None
        ringid   = ring[0]['ring_id']
        sourceid = ring[0]['sourceid'] 
        
        # Get the program id of the current source for the ring or return
        # none if there isn't one
        
        cursor = self._project.connection.cursor()
        cursor.execute('''
            SELECT program_id FROM data_sources WHERE ring_id = ?
                       ''', (ringid,))
        info = cursor.fetchone()
        if info is None:
            return None
        
        programId = info[0]
        
        #  Now use the program id to get information about the program itself
        # and build result dict:
        
        programInfo = self._programs.find(program_id=programId)
        return {
            'ring_id': ringid, 'sourceid': sourceid,
            'program_info': programInfo[0]
        }
        
        
        
    ##
    #  exists
    #    Checks for the existence of a data source 
    # @param ring - name of the ring
    # @param host - name of host in which the ring lives.
    #
    # @return boolean True if there is a source, False if not.
    def exists(self, ring, host):
        return self.ringSource(ring, host) is not None
    
    ##
    # isSource
    #   Determines if a program is a data source.
    #
    # @param program -program id.
    # @return Mixed
    # @retval None - The program is not a source.
    # @retval dict - Information about the ring on which it is a source
    #                if it is one.  The dict has same keys as
    #                Ring.find's return dict.
    #
    def isSource(self, program):
        cursor = self._project.connection.cursor()
        cursor.execute('''
            SELECT ring_id FROM data_sources WHERE program_id = ?
        ''', (program,))
        p = cursor.fetchone()
        
        if p is None:
            return None
        
        ringid   = p[0]
        ringinfo = self._rings._query((ringid,), '', 'WHERE ring_id=?')
        return ringinfo[0]
    
    ##
    # delete
    #    Given a source id, delete it.
    #
    # @param id - Id of the source.
    # @throw If the id does not correspond to a source.
    #
    def delete(self, id):
        c = self._project.connection.cursor()
        c.execute('''
            DELETE FROM data_sources WHERE id = ?
        ''', (id,))
        
        if c.rowcount == 0:
            raise RuntimeError(
                'project.DataSource.delete - no data source for %d' % id
            )
    ##
    # deleteRingSource
    #
    #  delete a ring's data source:
    #
    # @param ringId - the id of the ring whose data source will be deleted.
    #
    #  It is not an error in this case to call this for a ringId with no data
    #  source.
    #
    def deleteRingSource(self, ringId):
        self._project.connection.execute('''
            DELETE FROM data_sources WHERE ring_id = ?
        ''', (ringId,))

    ##
    # deleteNamedRingSource
    #
    #   Delete a source for the ring specified by name/host
    #
    # @param ringName - Name of the ring.
    # @param host     - Name of the host the ring lives in.
    #
    def deleteNamedRingSource(self, ringName, host):
        
        # Get the id of the ring.
        
        ring = self._rings.find(ringName, host)
        if len(ring) != 0:
            ringid   = ring[0]['ring_id']
            self.deleteRingSource(ringid)
 
#------------------------------------------------------------------------------

##
# @class Consumers
#
#   This class manages the consumers table.  A consumer is a program that
#   takes data from a ring.  Note that the ring may be remote or it
#   may be local (that is the program's host_id need not be the same as the
#   ring's host_id).
#
# This class supports the usual CRUD operations.  Where inserts involve
# multiple operations they are bundled in a transaction.
#

class Consumers:
    def __init__(self, project):
        self._project =  project
        self._rings   =  Rings(project)
        self._hosts   =  Hosts(project)
        self._programs = Programs(project)
        
    #-------------------------------------------------------------------------
    # Public entry points.
    
    ##
    # add
    #   Add a new consumer.  The assumption now is that a consumer only
    #   consumes data from a single ring (see throw list below).
    #
    # @param ring_id    - Id of the ring the data is being consumed from.
    # @param program_id - Id of the program that will consume data from
    #                     this ring.
    # @return integer -id of the consumer.
    #
    # @throw RunTimeError - The ring does not exist.
    # @throw RunTimeError - The program Id does not exist.
    #
    def add(self, ring_id, program_id):
        
        # Raise RuntimeError if the ring is bad
        
        if not self._rings._existsId(ring_id):
            raise RuntimeError(  
                '''project.Consumers.add - Ring %d does not exist''' %
                (ring_id,)
            )
        # Raise RuntimeError if the program Id is bad:
        
        if program_id not in self._programs.ids():
            raise RuntimeError(
                '''project.Consumers.add - Program %d does not exist''' %
                (program_id,)
            )
            
        # Raise runtime Error if the program Id is already a consumer.
        
        curr = self._project.connection.cursor()
        curr.execute(
            '''
            SELECT COUNT(*) from consumers WHERE program_id = ?
            ''', (program_id,)
        )
        if curr.fetchone()[0] > 0:
            raise RuntimeError(
                '''project.Consumers.add - Program %d is already a consumer''' %
                (program_id,)
            )
        # Everything checks out we can insert the record:
        
        curr.execute('''
            INSERT INTO consumers (ring_id, program_id) VALUES (?,?)
        ''', (ring_id, program_id))
        return curr.lastrowid
    
    ##
    # add_withNamedRing
    #
    #   Just translates the ringName/ringHost to a ringid and
    #   run add.   We'll intercept the error case where there's no ring
    #   so that our RuntimeError can have a better description of the problem.
    #
    # @param ringName - Name of the ring to consumer from.
    # @param ringHost - Host in which that ring lives.
    # @param program_id - program that will be a consumer.
    # @return integer   - id of the consumer added.
    #
    # @throw RuntimeError - if the ringName,ringHost don't translate to a ring id.
    #
    def add_withNamedRing(self, ringName, ringHost, program_id):
        ringInfo = self._rings.find(ringName, ringHost)
        if len(ringInfo) == 0:
            raise RuntimeError(
                'project.Consumers.add_withNamedRing - %s@%s does not exist' %
                (ringName, ringHost)
            )
        self.add(ringInfo[0]['ring_id'], program_id)
    ##
    # list
    #   Lists the consumers.  The table are joined with related tables
    #   to provide full information about the consumer
    #
    # @return list of dicts.  Each dict descdribes a consumer and has the
    #                         following information:
    #                         * id - Id of the consumer (used to delete e.g.).
    #                         * ring_info - Information about the ring. This is
    #                           a dict that contains the keys in Ring.list
    #                         * program_info -Information about the program.
    #                           this is a dict that contains information about
    #                           the program.  It contains the same keys as in
    #                           Programs.list
    #
    def list(self):
        result = list()
        conn             = self._project.connection
        oldFactory       = conn.row_factory
        conn.row_factory = sqlite3.Row
        cursor           = conn.cursor()
        
        cursor.execute('''
            SELECT c.id,
                    r.id AS ring_id, r.name AS ring_name, r.sourceid,
                    rh.id AS rhost_id, rh.host_name AS rhost_name,
                    p.id AS program_id, p.path AS path, p.working_dir AS working_dir,
                    ph.id AS phost_id, ph.host_name AS phost_name   
            FROM consumers c
            INNER JOIN rings r    ON r.id = c.ring_id
            INNER JOIN hosts rh   ON rh.id= r.host_id
            
            INNER JOIN programs p ON p.id = c.program_id
            INNER JOIN hosts ph   ON ph.id= p.host_id
                       '''
        )
        row = cursor.fetchone()
        while row is not None:
            info = dict()
            ringInfo = dict()
            progInfo = dict()
            
            info['id']             = row['id']
            
            ringInfo['ring_id']    = row['ring_id']
            ringInfo['ring_name']  = row['ring_name']
            ringInfo['sourceid']   = row['sourceid']
            ringInfo['host_id']    = row['rhost_id']
            ringInfo['host_name']  = row['rhost_name']
            
            progInfo['program_id']    = row['program_id']
            progInfo['path']          = row['path']
            progInfo['working_dir']   = row['working_dir']
            progInfo['host_id']       = row['phost_id']
            progInfo['host_name']     = row['phost_name']
            progInfo['args']          = self._programs._getArgs(row['program_id'])
            
            info['ring_info']    = ringInfo
            info['program_info'] = progInfo
            
            result.append(info)
            
            row = cursor.fetchone()
        
        conn.row_factory = oldFactory
        return result
    
    ##
    # consumers
    #   Provides a list of the consumers that are attached to a single ring.
    #   is is not an error for the ringName@ringHost to not exist, that just
    #   returns an empty list.
    #
    # @param ringName - the name of a ring.
    # @param ringHost - host in which the ring lives.
    #
    # @return list of dicts where each dict is a consumer and has the
    #               keys in Programs.list
    #
    def consumers(self, ringName, ringHost):
        result = list()
        
        # collect all the program ids:
        
        pids   = list()
        cursor = self._project.connection.cursor()
        cursor.execute('''
            SELECT program_id
                FROM consumers
                INNER JOIN  rings ON rings.id = consumers.ring_id
                INNER JOIN  hosts ON hosts.id = rings.host_id
                WHERE rings.name = ?
                AND   hosts.host_name = ?
                       ''', (ringName, ringHost))
        ids = list()
        row = cursor.fetchone()
        while row is not None:
            ids.append(str(row[0]))
            row = cursor.fetchone()
        
        #  If there are ids (consumers) use programs._query to find out about them.
        
        if len(ids) > 0:
            idList = ', '.join(ids)
            result = self._programs._query('WHERE p.id IN (%s)' % (idList,), ())
            return result
        else:
            return result
    
    ##
    # isConsumer
    #   @param id of a program.
    #   @return boolean
    #   @retval - True if the program is consuming from a ring.
    #   @retval - False if the program is not consuming from a ring.
    #
    #   @note if the program_id does not exist, False will be returned.
    #
    def isConsumer(self, program_id):
        curs = self._project.connection.cursor()
        curs.execute('''
            SELECT COUNT(*) FROM consumers WHERE program_id = ?
                     ''', (program_id,))
        r = curs.fetchone()
        return r[0] > 0

    ##
    #  delete
    #
    #   deletes a consumer relationship.
    #
    # @param id - the id of the consumer relationship to destroy.
    #
    # @throw RuntimeError - for deleting an id that does not exist.
    #
    def delete(self, id):
        cursor = self._project.connection.cursor()
        cursor.execute('''
            SELECT COUNT(*) FROM consumers WHERE id = ?       
        ''', (id,))
        r = cursor.fetchone()
        if r[0] == 0:
            raise RuntimeError(
                'project.Consumers.delete - No consumer with id %d' % (id,)
            )
        
        self._project.connection.execute('''
            DELETE FROM consumers WHERE id = ?
        ''', (id,)
        )
        
#-------------------------------------------------------------------------------
# Data loggers


##
# @class Loggers
#
#  Entries in the loggers table represent software that is dedicated to
#  logging event data from a ring.  A logger:
#  *  Takes data from a ring (is a special kind of consumer).
#  *  lives in a host.
#  *  May have a program id if the user is overiding the default logger program.
#  *  has a directory into which it is supposed to log the data.
#
#

class Loggers:
    def __init__(self, project):
        self._project  = project
        self._programs = Programs(project)
        self._hosts    = Hosts(project)
        self._rings    = Rings(project)
    #-------------------------------------------------------------------------
    # Private utility methods:
    
    
    ##
    # _getRingId
    #   Given a dict that either specifies a ringid or specifies
    #   a ringname/host returns the ring id.
    #
    # @param specs - The dict.  We care about the keywords:
    #               * ringId - the id of the ring.
    #               * ringName - the name of the ringbuffer.
    #               * ringHost - the host in which the ringbuffer lives.
    #
    # @return integer - id of the ring.
    #

    def _getRingId(self, specs):
        
        # We need to have either 'ringId' or the 'ringHost/'ringName' but not both
        
        if 'ringId' in specs.keys():
            if set(specs.keys()).isdisjoint(set(['ringName', 'ringHost'])):
                return specs['ringId']
            else:
                raise RuntimeError(
                    'project.Loggers._getRingId - ringId or {ringHost, ringName} can be specified but not both'
                )
        else:
            host = specs['ringHost']
            name = specs['ringName']
            ringInfo = self._rings.find(name, host)
            return ringInfo[0]['ring_id']
            
    ##
    # _getHostId
    #   Return the host id given  a specs dict that specifies either the host
    #   id or name.
    #
    # @param specs   - Specifications dict.  We care about the following keys
    #                  * loggerHostId   - Id of the host
    #                  * loggerHostName - Name of the host.l
    #
    # @return integer - id of the host
    #
    def _getHostId(self, specs):
        if 'loggerHostId' in specs.keys():
            return specs['loggerHostId']
        else:
            return self._hosts.id(specs['loggerHostName'])
    #--------------------------------------------------------------------------
    # Public methods
    
    ##
    # add
    #   Add an event logger to the system:
    #
    # @param ringId - the id of the ring from which data are logged.
    # @param hostId - The host in which the logger will run. If the program
    #                 id is specified, the host in the program id overrides
    # @param logPath - The path to which the data should be logged.
    # @param programId -If null the default logging program for NSCLDAQ is used.
    #                   if not this must be the id of an existing program
    #                   and that program will be used as the logger, run int the
    #                   host specified for it in the program table (overiding the
    #                   host specified here.
    #
    def add(self, ringId, hostId, logPath, programId = None):
        
        # Require the ringId be an existing ring.
        
        if not self._rings._existsId(ringId):
            raise RuntimeError(
                'project.Loggers.add - Ring id %d does not exist' % (ringId,)
            )
        # Require the host id be an existing host.
        
        if  not self._hosts._exists(hostId, 'id'):
            raise RuntimeError(
                'project.Loggers.add - Host id %d dos not exist' % (hostId,)
            )
             
        # If the program_id is not null, it must exist:
        
        if (programId is not None) and (programId not in self._programs.ids()):
            raise RuntimeError(
                'project.Loggers.add - Program id %d does not exist' % (programId,)
            )
        
        # Everything checks out so do the insert.
        
        self._project.connection.execute(
        '''
        INSERT INTO loggers (ring_id, host_id, program_id, log_dir)
            VALUES(?,?,?,?)
        ''', (ringId, hostId, programId, logPath)
    )
    ##
    # add_Specified
    #   Add which uses keyword/value pairs to specify the eventlogger to add.
    #
    # @param **specs - Keyword=value pairs.  The valid keywords are:
    #                 * ringId         - Id of the ring from which data are
    #                                    taken.
    #                 * ringName       - Name of the ring from which data are
    #                                    taken.
    #                 * ringHost       - Host in which the ring lives.
    #                 * loggerHostId   - Id of the host in which the log program
    #                                    runs
    #                 * loggerHostName - Name of an existing host in which the log
    #                                    program will run.
    #                 * logPath        - Directory in which the data will be logged.
    #                 * programId      - If supplied the id of a program that will
    #                                    be used instead of the default system
    #                                    logger.
    #
    #  @throw RuntimeError - in the event of inconsistent uses of the keywords.
    #  @throw RuntimeError - see add above for additional causes.
    #
    #  @note The use of keywords is not completely unconstrained:
    #        * If ringId is used, then neither ringName nor ringHost are allowed.
    #        * If ringName or ringHost are used then ringId is not allowed
    #          and both ringName and ringHost must be present, and resolve
    #          to a ring id.
    #        * Only one of loggerHostId and loggerHostName can be used.
    #
    def add_Specified(self, **specs):
        ringId = self._getRingId(specs)
        hostId = self._getHostId(specs)
        self.add(ringId, hostId, specs['logPath'], None)
    
    def list(self):
        pass
    
    def find(self, **specs):
        pass
    
    def delete(self, id):
        pass