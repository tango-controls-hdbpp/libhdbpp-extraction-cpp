#include "mysql/mysqlrow.h"
#include <stdlib.h>

MySqlRow::MySqlRow(MYSQL_RES *res, MYSQL_ROW row)
{
    mResult = res;
    mFields = row;
}

MySqlRow::~MySqlRow()
{
}

char *MySqlRow::getField(int i) const
{
    if(mFields == NULL)
        return NULL;
    if(i >= 0 && i < getFieldCount())
        return mFields[i];
    return NULL;
}

/** \brief Gets the number of fields of this row, or -1 if the row is closed or invalid
 *
 */
int MySqlRow::getFieldCount() const
{
    if(mFields == NULL)
        return -1;
    return mysql_num_fields(mResult);
}

bool MySqlRow::isClosed() const
{
    return mFields == NULL;
}

void MySqlRow::close()
{
    mFields = NULL;
    mResult = NULL;
}

