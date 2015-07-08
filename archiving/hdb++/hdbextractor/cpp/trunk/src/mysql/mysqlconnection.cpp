#include <stdlib.h> /* for NULL */
#include <stdio.h>
#include <string.h>
#include "mysql/mysqlconnection.h"
#include "mysqlresult.h"
#include "mysqlrow.h"
#include "xvariant.h"
#include "hdbxmacros.h"

#define MAXQUERYLEN 256

MySqlConnection::MySqlConnection()
{
    mConnection = NULL;
    mAffectedRows = 0;
}

MySqlConnection:: ~MySqlConnection()
{
    close();
}

void MySqlConnection::close()
{
    if(mConnection != NULL)
    {
        mysql_close(mConnection);
        mConnection = NULL;
    }
}

bool MySqlConnection::connect(const char* host,
                              const char *db,
                              const char* user,
                              const char* passwd,
                              unsigned short port)
{
    mysql_init(&mMysql);
    mConnection = mysql_real_connect(&mMysql, host, user, passwd, db, port, 0, 0);
    printf("\e[1;35m connection %p %s %s %s %s\e[0m\n", mConnection, host, user, passwd, db);
    return (mConnection != NULL);
}

int MySqlConnection::getAffectedRows() const
{
    return mAffectedRows;
}

const char* MySqlConnection::getError()
{
    if(isConnected())
        return mysql_error(mConnection);
    else
        return mysql_error(&mMysql);
}

bool MySqlConnection::isConnected() const
{
    return mConnection != NULL;
}

Result *MySqlConnection::query(const char *q)
{
    int res;
    MYSQL_RES* result;

    if(mConnection == NULL)
    {
        perr("MySqlConnection::query: database not open");
    }
    else
    {
        res = mysql_query(mConnection, q);
        if(res != 0)
            return NULL;
        else
        {
            result = mysql_store_result(mConnection);
            if(!result)
            {
                int field_count = mysql_num_fields(result);
                if(field_count != 0) /* an error occurred */
                {
                    return NULL;
                }
                else
                {
                    /* store the affected rows */
                    mAffectedRows = mysql_affected_rows(mConnection);
                }
                return NULL;
            }
            return new MySqlResult(result);
        }
    }
}

