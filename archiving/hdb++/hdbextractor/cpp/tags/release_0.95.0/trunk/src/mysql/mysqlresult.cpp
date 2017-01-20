#include "mysqlresult.h"
#include "mysqlrow.h"
#include <stdlib.h>
#include "hdbxmacros.h"

MySqlResult::MySqlResult(MYSQL_RES *mysqlResult) : Result()
{
    mResult = mysqlResult;
    dRow = NULL;
    dRowCount = 0;
}

MySqlResult::~MySqlResult()
{
    close();
}

void MySqlResult::close()
{
    if(mResult != NULL)
    {
        mysql_free_result(mResult);
        mResult = NULL;
    }
}

Row *MySqlResult::getCurrentRow() const
{
    if(mResult == NULL)
        return NULL;
    else
        return dRow;
}

int MySqlResult::getRowCount()
{
    if(mResult == NULL)
        return -1;
    else
        dRowCount = mysql_num_rows(mResult);
    return dRowCount;
}

int MySqlResult::next()
{
    MYSQL_ROW row;
    if(mResult == NULL)
    {
        return 0;
    }
    else
    {
        row = mysql_fetch_row(mResult);

        if(!row)
        {
            dRow = NULL;
            return 0;
        }
        else
        {
            dRow = new MySqlRow(mResult, row);
            return 1;
        }
    }
}
