
package require vardb

proc MakeDirectory {dbname path} {
    set handle [vardb::open $dbname]
    ::vardb::mkdir $handle $path

    return $handle
}

MakeDirectory myvariables.db "/example/mydir"
