/**

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
# @file   CSqliteTransaction.cpp
# @brief  Implement auto-commit transaction and its associated exception.
# @author <fox@nscl.msu.edu>
*/

#include "CSqliteTransaction.h"
#include "CSqlite.h"
#include "CSqliteStatement.h"

/*----------------------------------------------------------------------------*/
/**
 * Implement the exception class used by the transaction methods:
 */

/**
 * constructor
 *  @param message - the error message
 */
CSqliteTransaction::CException::CException(std::string message) noexcept :
    m_message(message) {}
    
/**
 * copy constructor
 * @param rhs - the exception from which we are constructed.
 */
CSqliteTransaction::CException::CException(const CException& rhs) noexcept :
    m_message(rhs.m_message) {}

/**
 * assignmentCSq
 *  @param rhs - The object being assigned to this.
 *  @return *this
 */
CSqliteTransaction::CException&
CSqliteTransaction::CException::operator=(const CException& rhs) noexcept 
{
    m_message = rhs.m_message;
    return *this;
}

/**
 * what
 *  @return const char*  - the error message string.
 */
const char*
CSqliteTransaction::CException::what() const noexcept
{
    return m_message.c_str();
}

/*----------------------------------------------------------------------------*/
/**
 * Implement the CSqliteTransaction class itself.
 */

/**
 * constructor
 *    Save the database, set the transaction state to active,
 *    and exectue a BEGIN TRANSACTION
 * @param db - The database on which the transaction is starting:
 */
CSqliteTransaction::CSqliteTransaction(CSqlite& db) :
    m_db(db), m_state(active)
{
    CSqliteStatement::execute(m_db, "BEGIN DEFERRED TRANSACTION");
}
/**
 * destructor
 *    Dispose of the transaction properly:
 *    - Active COMMIT
 *    - rollbackpending ROLLBACK
 *    - completed do nothing.
 */
CSqliteTransaction::~CSqliteTransaction()
{
    if (m_state == active) {
        commit();
    } else if (m_state == rollbackpending) {
        rollback();
    }

}
/**
 * rollback
 *    Rollback the transaction now.  The state is set to completed so
 *    destruction is a no-op.
 */
void CSqliteTransaction::rollback()
{
    switch(m_state) {
        case active:
        case rollbackpending:
            CSqliteStatement::execute(m_db, "ROLLBACK TRANSACTION");
            m_state = completed;
            break;
        case completed:
            throw CException("Rollback attemped on completed transaction");
            break;
    }
}
/**
 * scheduleRollback
 *   Mark the transaction to be rolled back on destruction.
 */
void CSqliteTransaction::scheduleRollback()
{
    switch(m_state) {
        case active:
            m_state = rollbackpending;
        case rollbackpending:
            break;
        case completed:
            throw CException("Attempted scheduleRollback on completed transaction");
    }
    
}
/**
 * commit
 *    Do a commit now.  The state of the transaction is set to completed so
 *    the destructor is a no-op.
 */
void CSqliteTransaction::commit()
{
    switch (m_state) {
        case active:
            CSqliteStatement::execute(m_db, "COMMIT TRANSACTION");
            m_state = completed;
            break;
        case completed:
            throw CException("Attempting to commmit a completed transaction");
            break;
        case rollbackpending:
            throw CException("Attempting to commit when rollback is pending");
            break;
    }
}

