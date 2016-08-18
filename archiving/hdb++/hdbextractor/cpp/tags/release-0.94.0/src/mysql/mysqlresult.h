#ifndef MYSQLRESULT_H
#define MYSQLRESULT_H

#include <result.h>
#include <mysql/mysql.h>

class MySqlRow;

/** \brief An implementation of Result specific to MySql database. <em>Used internally</em>.
 *
 */
class MySqlResult : public Result
{
public:
    MySqlResult(MYSQL_RES *mysqlResult);

    virtual ~MySqlResult();

    virtual void close();

    virtual Row *getCurrentRow() const;

    virtual int getRowCount();

    virtual int next();

private:
    MYSQL_RES *mResult;
};

#endif // MYSQLRESULT_H
