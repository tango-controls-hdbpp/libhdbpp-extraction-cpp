#ifndef RESULT_H
#define RESULT_H

#include "row.h"

/** \brief Defines an interface for a result of the database. <em>Used internally</em>.
 *
 * This interface defines the interaction with a result obtained from a database.
 * It must be possible to get the current row, the row count, and to move to the next row.
 *
 * <em>Used internally</em>.
 */
class Result
{
public:
    Result();

    virtual ~Result();

    virtual void close() = 0;

    virtual Row* getCurrentRow() const = 0;

    virtual int getRowCount() = 0;

    virtual int next() = 0;

protected:

    int dRowCount;
    Row *dRow;
};

#endif // RESULT_H
