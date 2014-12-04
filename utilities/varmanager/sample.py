import nscldaq.vardb.vardb
import nscldaq.vardb.dirtree


def MakeDirectory(dbFile, path):
    myVarDb = nscldaq.vardb.vardb.VarDb(dbFile)
    myDirTree = nscldaq.vardb.dirtree.DirTree(myVarDb)
    myDirTree.mkdir(path)
    return (myVarDb, myDirTree)


if __name__ == '__main__':
    MakeDirectory('myvariables.db', '/example/mydir')
