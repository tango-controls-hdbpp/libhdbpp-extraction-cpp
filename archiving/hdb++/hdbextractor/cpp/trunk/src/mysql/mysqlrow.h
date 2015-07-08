#ifndef MYSQLROW_H
#define MYSQLROW_H

#include <mysql/mysql.h>
#include <row.h>

/** \brief An implementation of the Row interface dealing with mysql results. <em>Used internally</em>.
 *
 */
class MySqlRow : public Row
{
public:
    MySqlRow(MYSQL_RES *mysqlRes, MYSQL_ROW row);

    virtual ~MySqlRow();

    virtual char *getField(int) const;

    virtual int getFieldCount() const;

    virtual bool isClosed() const;

    virtual void close();

private:
    MYSQL_RES *mResult;
    MYSQL_ROW mFields;
};

#endif // MYSQLROW_H
