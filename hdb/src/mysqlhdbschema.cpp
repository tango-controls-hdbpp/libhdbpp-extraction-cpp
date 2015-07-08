#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "mysqlhdbschema.h"
#include "db/src/connection.h"
#include "db/src/row.h"
#include "db/src/result.h"
#include "mysql/src/mysqlconnection.h"
#include "../src/hdbxmacros.h"
#include "db/src/dbschemaprivate.h"
#include "xvariantlist.h"
#include <timeinterval.h>
#include <assert.h>
#include <map>

#define MAXQUERYLEN 4096
#define MAXTABLENAMELEN 32
#define MAXTIMESTAMPLEN 64

MySqlHdbSchema::MySqlHdbSchema(ResultListener *resultListenerI) : ConfigurableDbSchema()
{
    assert(resultListenerI != NULL);
    /* d_ptr is created inside ConfigurableDbSchema */
    d_ptr->resultListenerI = resultListenerI;
    d_ptr->variantList = NULL;
    d_ptr->sourceStep = 1;
    d_ptr->totalSources = 1;
    pthread_mutex_init(&d_ptr->mutex, NULL);
}

const char *MySqlHdbSchema::getError() const
{
    return d_ptr->errorMessage;
}

bool MySqlHdbSchema::hasError() const
{
    return strlen(d_ptr->errorMessage) > 0;
}

/** \brief The class destructor.
 *
 * Deallocates the mutex used for thread safety.
 */
MySqlHdbSchema::~MySqlHdbSchema()
{
    pthread_mutex_destroy(&d_ptr->mutex);
}

/** \brief empties the queue of partial or complete data already fetched from the database.
 *
 * @param variantlist a <strong>reference</strong> to a std::vector where data is copied.
 *
 * \note The caller is not in charge of freeing any memory used by MySqlHdbSchema. The caller
 *       creates and manages the variantlist.
 */
int MySqlHdbSchema::get(std::vector<XVariant>& variantlist)
{
    pthread_mutex_lock(&d_ptr->mutex);

    int size = -1;
    if(d_ptr->variantList != NULL)
    {
        size = (int) d_ptr->variantList->size();

        printf("\e[0;35mMySqlHdbSchema.get: locketh xvarlist for writing... size %d \e[0m\t", size);

        for(int i = 0; i < size; i++)
        {
            //            printf("copying variant %d over %d\n", i, size);
            variantlist.push_back(XVariant(*(d_ptr->variantList->get(i))));
        }
        delete d_ptr->variantList;
        d_ptr->variantList = NULL;
    }

    pthread_mutex_unlock(&d_ptr->mutex);
    printf("\e[0;32munlocked: [copied %d]\e[0m\n", size);
    return size;
}

bool MySqlHdbSchema::getData(const char *source,
                                const TimeInterval *time_interval,
                                Connection *connection,
                                int notifyEveryRows)
{
    return getData(source, time_interval->start(), time_interval->stop(), connection, notifyEveryRows);
}

bool MySqlHdbSchema::getData(const std::vector<std::string> sources,
                                const TimeInterval *time_interval,
                                Connection *connection,
                                int notifyEveryRows)
{
    return getData(sources, time_interval->start(), time_interval->stop(), connection, notifyEveryRows);
}

/** \brief Fetch attribute data from the database between a start and stop date/time.
 *
 * Fetch data from the database.
 * \note This method is used by HdbExtractor and it is not meant to be directly used by the library user.
 *
 * @param source A the tango attribute in the form domain/family/member/AttributeName
 * @param start_date the start date (begin of the requested data interval) as string, such as "2014-07-10 10:00:00"
 * @param stop_date the stop date (end of the requested data interval) as string, such as "2014-07-10 12:00:00"
 * @param connection the database Connection specific object
 * @param notifyEveryRows the number of rows that make up a block of data. Every time a block of data is complete
 *        notifications are sent to the listener of type ResultListener (HdbExtractor)
 *
 * @return true if the call was successful, false otherwise.
 */
bool MySqlHdbSchema::getData(const char *source,
                             const char *start_date,
                             const char *stop_date,
                             Connection *connection,
                             int notifyEveryNumRows)
{
    bool success, from_the_past_success = true;
    char query[MAXQUERYLEN];
    char errmsg[256];
    char ch_id[16];
    char data_type[16];
    char data_format[16];
    char writable[16];
    char table_name[64];
    int id;
    int rowCnt = 0;
    double elapsed = -1.0; /* query elapsed time in seconds.microseconds */
    double from_the_past_elapsed = 0.0;
    struct timeval tv1, tv2;

    gettimeofday(&tv1, NULL);

    /* clear error */
    strcpy(d_ptr->errorMessage, "");

    d_ptr->notifyEveryNumRows = notifyEveryNumRows;

    snprintf(query, MAXQUERYLEN, "SELECT ID,data_type,data_format,writable from adt WHERE full_name='%s'", source);

    printf("\e[1;4;36mHDB: query %s\e[0m\n", query);

    Result * res = connection->query(query);
    if(!res)
    {
        snprintf(d_ptr->errorMessage, MAXERRORLEN,
                 "MysqlHdbSchema.getData: error in query \"%s\": \"%s\"", query, connection->getError());
        return false;
    }
    if(res->next() > 0)
    {
        Row* row = res->getCurrentRow();
        if(!row)
        {
            snprintf(d_ptr->errorMessage, MAXERRORLEN, "MysqlHdbSchema.getData: error getting row %d", rowCnt);
            return false;
        }

        if(row->getFieldCount() == 4)
        {
            strncpy(ch_id, row->getField(0), 16);
            strncpy(data_type, row->getField(1), 16);
            strncpy(data_format, row->getField(2), 16);
            strncpy(writable, row->getField(3), 16);
            snprintf(table_name, MAXTABLENAMELEN, "att_%05d", atoi(ch_id));
            /* free memory */
            res->close();
            row->close();

            /*
                 * enum AttributeDataType { ATT_BOOL, ATT_SHORT, ATT_LONG, ATT_LONG64, ATT_FLOAT,
                 * ATT_DOUBLE, ATT_UCHAR, ATT_USHORT, ATT_ULONG, ATT_ULONG64, ATT_STRING, ATT_STATE, DEVICE_STATE,
                 * ATT_ENCODED, NO_DATA ...
                 */
            XVariant::DataType dataType;
            if(strcmp(data_type, "5") == 0)
                dataType = XVariant::Double;
            else if(strcmp(data_type, "4") == 0)
                dataType = XVariant::Double;
            else if(strcmp(data_type, "0") == 0)
                dataType = XVariant::Boolean;
            else if(strcmp(data_type, "1") == 0) /* short */
                dataType = XVariant::Int;
            else if(strcmp(data_type, "2") == 0) /* long */
                dataType = XVariant::Int;
            else
                dataType = XVariant::TypeInvalid;

            XVariant::Writable wri;
            if(!strcmp(writable, "0"))
                wri = XVariant::RO;
            else if(!strcmp(writable, "3"))
                wri = XVariant::RW;
            else
                wri = XVariant::WritableInvalid;

            XVariant::DataFormat format;
            if(!strcmp(data_format, "0"))
                format = XVariant::Scalar;
            else if(!strcmp(data_format, "1"))
                format = XVariant::Vector;
            else if(!strcmp(data_format, "2"))
                format = XVariant::Matrix;
            else
                format = XVariant::FormatInvalid;

            if(dataType == XVariant::TypeInvalid || wri ==  XVariant::WritableInvalid ||
                    format == XVariant::FormatInvalid)
            {
                snprintf(d_ptr->errorMessage, MAXERRORLEN,
                         "MySqlHdbSchema.getData: invalid type %d, format %d or writable %d",
                         dataType, format, wri);
                success = false;
            }
            else
            {
                const ConfigurableDbSchemaHelper *configHelper = new ConfigurableDbSchemaHelper();
                ConfigurableDbSchemaHelper::FillFromThePastMode fillMode = ConfigurableDbSchemaHelper::None;
                /* now get data */
                id = atoi(ch_id);
                if(wri == XVariant::RO)
                {
                    snprintf(query, MAXQUERYLEN, "SELECT time,value FROM %s WHERE time >='%s' "
                                                 " AND time <= '%s' ORDER BY time ASC", table_name, start_date, stop_date);

                    printf("\e[1;4;36mHDB: query %s\e[0m\n", query);

                    res = connection->query(query);
                    if(!res)
                    {
                        snprintf(d_ptr->errorMessage, MAXERRORLEN, "error in query \"%s\": \"%s\"",
                                 query, connection->getError());
                        return false;
                    }
                    while(res->next() > 0)
                    {
                        row = res->getCurrentRow();
                        if(!row)
                        {
                            snprintf(d_ptr->errorMessage, MAXERRORLEN, "MysqlHdbSchema.getData: error getting row %d", rowCnt);
                            return false;
                        }

                        if(rowCnt == 0)
                        {
                            if(configHelper->fillFromThePastMode(d_ptr->queryConfiguration,
                                                                 start_date,
                                                                 stop_date,
                                                                 row->getField(0))
                                    != ConfigurableDbSchemaHelper::None)
                            {
                                from_the_past_success = fetchInThePast(source, start_date, table_name, id,
                                                                       dataType, format, wri, connection,
                                                                       &from_the_past_elapsed,
                                                                       fillMode);
                                if(from_the_past_success)
                                    rowCnt++;
                            }
                        }

                        rowCnt++;

                        XVariant *xvar = NULL;
                    //    printf("+ adding %s %s (row count %d)\n", row->getField(0), row->getField(1), res->getRowCount());

                        xvar = new XVariant(source, row->getField(0), row->getField(1), format, dataType, wri);

                        pthread_mutex_lock(&d_ptr->mutex);
                        if(d_ptr->variantList == NULL)
                            d_ptr->variantList = new XVariantList();

                        d_ptr->variantList->add(xvar);
                        pthread_mutex_unlock(&d_ptr->mutex);

                        row->close();

                        if(d_ptr->notifyEveryNumRows > 0 && (rowCnt % d_ptr->notifyEveryNumRows == 0
                                                             || rowCnt == res->getRowCount()) )
                        {
                            d_ptr->resultListenerI->onProgressUpdate(source, rowCnt, res->getRowCount());
                        }
                    } /* res is closed at the end of else if(wri == XVariant::RW) */

                    success = true;
                }
                else if(wri == XVariant::RW)
                {
                    /*  */
                    snprintf(query, MAXQUERYLEN, "SELECT time,read_value,write_value FROM %s WHERE time >='%s' "
                                                 " AND time <= '%s' ORDER BY time ASC", table_name, start_date, stop_date);

                    printf("\e[1;4;36mHDB: query %s\e[0m\n", query);

                    res = connection->query(query);
                    if(!res)
                    {
                        snprintf(d_ptr->errorMessage, MAXERRORLEN, "error in query \"%s\": \"%s\"",
                                 query, connection->getError());
                        return false;
                    }
                    while(res->next() > 0)
                    {
                        row = res->getCurrentRow();
                        if(!row)
                        {
                            snprintf(d_ptr->errorMessage, MAXERRORLEN, "MysqlHdbSchema.getData: error getting row %d", rowCnt);
                            return false;
                        }

                        if(rowCnt == 0)
                        {
                            if(configHelper->fillFromThePastMode(d_ptr->queryConfiguration,
                                                                 start_date,
                                                                 stop_date,
                                                                 row->getField(0))
                                    != ConfigurableDbSchemaHelper::None)
                            {
                                from_the_past_success = fetchInThePast(source, start_date, table_name, id,
                                                                       dataType, format, wri, connection,
                                                                       &from_the_past_elapsed,
                                                                       fillMode);
                                if(from_the_past_success)
                                    rowCnt++;
                            }
                        }

                        rowCnt++;

                        XVariant *xvar = NULL;
                        // printf("+ adding %s %s (row count %d)\n", row->getField(0), row->getField(1), res->getRowCount());

                        xvar = new XVariant(source, row->getField(0), row->getField(1),
                                            row->getField(2), format, dataType);

                        pthread_mutex_lock(&d_ptr->mutex);
                        if(d_ptr->variantList == NULL)
                            d_ptr->variantList = new XVariantList();
                        d_ptr->variantList->add(xvar);
                        pthread_mutex_unlock(&d_ptr->mutex);

                        row->close();

                        if(d_ptr->notifyEveryNumRows > 0 && (rowCnt % d_ptr->notifyEveryNumRows == 0
                                                             || rowCnt == res->getRowCount()) )
                        {
                            d_ptr->resultListenerI->onProgressUpdate(source, rowCnt, res->getRowCount());
                        }
                    } /* end while(res->next() > 0) res is closed at the end */

                    success = from_the_past_success;
                }

                if(res && res->getRowCount() == 0)
                {
                    printf("\e[1;36mno rows. Getting from the past\e[0m\n");
                    if(configHelper->fillFromThePastMode(d_ptr->queryConfiguration,
                                                         start_date,
                                                         stop_date,
                                                         "")
                            != ConfigurableDbSchemaHelper::None)
                    {
                        from_the_past_success = fetchInThePast(source, start_date, table_name, id,
                                                               dataType, format, wri, connection,
                                                               &from_the_past_elapsed,
                                                               fillMode);
                        if(from_the_past_success)
                            rowCnt++;
                    }
                }

                if(res)
                    res->close();


                delete configHelper;

            } /* else: valid data type, format, writable */
        }
    }
    else
    {
        success = false;
        snprintf(d_ptr->errorMessage, MAXERRORLEN, "MysqlHdbSchema: no attribute \"%s\" in adt", source);
        perr(errmsg);
    }

    /* compute elapsed time */
    gettimeofday(&tv2, NULL);
    /* transform the elapsed time from a timeval struct to a double whose integer part
     * represents seconds and the decimal microseconds.
     */
    elapsed = tv2.tv_sec + 1e-6 * tv2.tv_usec - (tv1.tv_sec + 1e-6 * tv1.tv_usec);
    /* source step is initialized to 1. It's changed by the std::vector<std::string> getData
     * method below.
     */
    d_ptr->resultListenerI->onFinished(source, d_ptr->sourceStep, d_ptr->totalSources, elapsed);

    return success;
}

/** \brief Fetch attribute data from the mysql hdb database between a start and stop date/time.
 *
 * Fetch data from the MySql hdb database.
 * \note This method is used by HdbExtractor and it is not meant to be directly used by the library user.
 *
 * @param source A the tango attribute in the form domain/family/member/AttributeName
 * @param start_date the start date (begin of the requested data interval) as string, such as "2014-07-10 10:00:00"
 * @param stop_date the stop date (end of the requested data interval) as string, such as "2014-07-10 12:00:00"
 * @param connection the database Connection specific object
 * @param notifyEveryRows the number of rows that make up a block of data. Every time a block of data is complete
 *        notifications are sent to the listener of type ResultListener (HdbExtractor)
 *
 * @return true if the call was successful, false otherwise.
 */
bool MySqlHdbSchema::getData(const std::vector<std::string> sources,
                             const char *start_date,
                             const char *stop_date,
                             Connection *connection,
                             int notifyEveryNumRows)
{
    bool success = true;
    d_ptr->totalSources = sources.size();
    for(size_t i = 0; i < d_ptr->totalSources; i++)
    {
        d_ptr->sourceStep = i + 1;
        printf("MySqlHdbSchema.getData %s %s %s\n", sources.at(i).c_str(), start_date, stop_date);
        success = getData(sources.at(i).c_str(), start_date, stop_date,
                          connection, notifyEveryNumRows);
        if(!success)
            break;
    }

    d_ptr->totalSources = 1;

    return success;

}

bool MySqlHdbSchema::getSourcesList(Connection *connection, std::list<std::string>& result) const
{
    return findSource(connection, "", result);
}

/** \brief The hdb schema does not support errors. This method will return false and does nothing.
 *
 * @return false This method always returns false and does nothing else.
 *
 */
bool MySqlHdbSchema::findErrors(const char *, const TimeInterval *,
                        Connection *) const
{
    perr("MySqlHdbSchema.findErrors: errors aren't saved into the hdb database");
    return false;
}

bool MySqlHdbSchema::findSource(Connection *connection, const char *substring, std::list<std::string>& result) const
{
    bool success = true;
    char query[MAXQUERYLEN];
    Row *row;

    snprintf(query, MAXQUERYLEN, "SELECT full_name from adt WHERE full_name like '%%%s%%'", substring);

    printf("\e[1;34mQUERY %s\e[0m\n", query);
    Result * res = connection->query(query);
    if(!res)
    {
        snprintf(d_ptr->errorMessage, MAXERRORLEN,
                 "MysqlHdbSchema.getSourcesList: error in query \"%s\": \"%s\"", query, connection->getError());
        success = false;
    }
    else
    {
        while(res->next() > 0 && success)
        {
            row = res->getCurrentRow();
            if(!row)
            {
                snprintf(d_ptr->errorMessage, MAXERRORLEN, "MysqlHdbSchema.getSourcesList: error getting row");
                success = false;
            }
            else
            {
                result.push_back(std::string(row->getField(0)));
            }
        }
    }
    return success;
}

bool MySqlHdbSchema::fetchInThePast(const char *source,
                                    const char *start_date, const char *table_name,
                                    const int /* att_id */,
                                    XVariant::DataType dataType,
                                    XVariant::DataFormat format,
                                    XVariant::Writable writable,
                                    Connection *connection,
                                    double *time_elapsed,
                                    ConfigurableDbSchemaHelper::FillFromThePastMode mode)
{
    char query[MAXQUERYLEN];
    char timestamp[MAXTIMESTAMPLEN];
    struct timeval tv1, tv2;
    Result *res = NULL;
    Row *row = NULL;

    gettimeofday(&tv1, NULL);

    printf("\e[1;4;36mHDB: fetching in the past \"%s\" before %s\e[0m\n", source, start_date);
    if(writable != XVariant::RW)
    {
        snprintf(query, MAXQUERYLEN, "SELECT time,value FROM "
                                     " %s WHERE time <= '%s' "
                                     " ORDER BY time DESC LIMIT 1",
                 table_name, start_date);
    }
    else
    {
        snprintf(query, MAXQUERYLEN, "SELECT time,read_value,write_value FROM "
                                     " %s WHERE time <= '%s' "
                                     " ORDER BY time DESC LIMIT 1",
                 table_name,  start_date);
    }

    printf("\e[1;36mHDB: query: %s\e[0m\n", query);
    res = connection->query(query);
    if(!res)
    {
        snprintf(d_ptr->errorMessage, MAXERRORLEN, "MySqlHdbSchema.fetchInThePast: bad query \"%s\": \"%s\"",
                 query, connection->getError());
        return false;
    }

    while(res->next() > 0)
    {
        row = res->getCurrentRow();

        if(!row)
        {
            snprintf(d_ptr->errorMessage, MAXERRORLEN, "MySqlHdbSchema.fetchInThePast: error getting row");
            return false;
        }
        else
        {
            XVariant *xvar = NULL;
            /* choose timestamp according to ConfigurableDbSchemaHelper mode */
            if(mode == ConfigurableDbSchemaHelper::KeepWindow)
                strncpy(timestamp, start_date, MAXTIMESTAMPLEN);
            else
                strncpy(timestamp, row->getField(0), MAXTIMESTAMPLEN);

            if(writable != XVariant::RW)
                xvar = new XVariant(source, timestamp, row->getField(1), format, dataType, writable);
            else
                xvar = new XVariant(source, timestamp, row->getField(1),
                                    row->getField(2), format, dataType);

            pthread_mutex_lock(&d_ptr->mutex);
            if(d_ptr->variantList == NULL)
                d_ptr->variantList = new XVariantList();
            d_ptr->variantList->add(xvar);
            pthread_mutex_unlock(&d_ptr->mutex);

            row->close();
        }
    }

    if(time_elapsed)
    {
        /* compute elapsed time */
        gettimeofday(&tv2, NULL);
        /* transform the elapsed time from a timeval struct to a double whose integer part
         * represents seconds and the decimal microseconds.
         */
        *time_elapsed = tv2.tv_sec + 1e-6 * tv2.tv_usec - (tv1.tv_sec + 1e-6 * tv1.tv_usec);
    }
    return true;
}



