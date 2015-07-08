#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "mysqlhdbschema.h"
#include "../db/connection.h"
#include "../db/row.h"
#include "../db/result.h"
#include "../mysql/mysqlconnection.h"
#include "../hdbxmacros.h"
#include "../db/dbschemaprivate.h"
#include "../db/xvariantlist.h"
#include "../db/timeinterval.h"
#include "../sharedpointer.h"
#include <assert.h>
#include <map>
#include <math.h>
#include "../hdbxsettings.h"

#define MAXQUERYLEN 4096
#define MAXTABLENAMELEN 32
#define MAXTIMESTAMPLEN 64

MySqlHdbSchema::MySqlHdbSchema(ResultListener *resultListenerI) : ConfigurableDbSchema()
{
    assert(resultListenerI != NULL);
    /* d_ptr is created inside ConfigurableDbSchema */
    d_ptr->resultListenerI = resultListenerI;
    d_ptr->variantList = NULL;
    d_ptr->totalRowCnt = 0;
    d_ptr->isCancelled = false;
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

      //  printf("\e[0;35mMySqlHdbSchema.get: locketh xvarlist for writing... size %d \e[0m\t", size);

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
  // The following is to test the SharedPointer for the source name */
  //  for(int i = 0; i < size; i++)
  //      printf("\e[1;33m source is %s (%p)\e[0m\n", variantlist.at(i).getSource(), variantlist.at(i).getSource());
    return size;
}

bool MySqlHdbSchema::getData(const char *source,
                                const TimeInterval *time_interval,
                                Connection *connection,
                                int notifyEveryPercent)
{
    return getData(source, time_interval->start(), time_interval->stop(), connection, notifyEveryPercent);
}

bool MySqlHdbSchema::getData(const std::vector<std::string> sources,
                                const TimeInterval *time_interval,
                                Connection *connection,
                                int notifyEveryPercent)
{
    return getData(sources, time_interval->start(), time_interval->stop(), connection, notifyEveryPercent);
}


/** \brief A getData version for internal use or for custom implementations who want to deal
 *         with custom extraction of multiple sources
 *
 * This method, with the additional parameters  sourceIndex and
 * totalSources, is intended for special use and does not reset the
 * cancelled flag. If you happen to use that version of getData, be sure to
 * call resetCancelledFlag before.
 *
 * @see resetCancelledFlag
 *
 */
bool MySqlHdbSchema::getData(const char *source,
              const char *start_date,
              const char *stop_date,
              Connection *connection,
              int notifyEveryPercent,
              int sourceIndex,
              int totalSources,
              double *elapsed)
{
    bool success;
    int rows_from_the_past = 0;
    char query[MAXQUERYLEN];
    char ch_id[16];
    char data_type[16];
    char data_format[16];
    char writable[16];
    char table_name[64];
    int id;
    int rowCnt = 0;
    double from_the_past_elapsed = 0.0;
    *elapsed = 0.0;
    struct timeval tv1, tv2;
    memset(&tv1, 0, sizeof(struct timeval));
    memset(&tv2, 0, sizeof(struct timeval));

    double myPercent = 100.0 / totalSources;
    int notifyEverySteps = -1;

    gettimeofday(&tv1, NULL);

    /* clear error */
    strcpy(d_ptr->errorMessage, "");

    d_ptr->notifyEveryPercent = notifyEveryPercent;

    snprintf(query, MAXQUERYLEN, "SELECT ID,data_type,data_format,writable from adt WHERE full_name='%s'", source);

    pinfo("HDB: query %s", query);

    Result * res = connection->query(query);
    if(!res)
    {
        snprintf(d_ptr->errorMessage, MAXERRORLEN,
                 "MysqlHdbSchema.getData: error in query \"%s\": \"%s\"", query, connection->getError());
        return false;
    }

    if(!d_ptr->isCancelled && res->next() > 0)
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
            else if(strcmp(data_type, "1") == 0)
                dataType = XVariant::Boolean;
            else if(strcmp(data_type, "2") == 0
                    || strcmp(data_type, "3") == 0 ) /* short or long */
                dataType = XVariant::Int;
            else if(strcmp(data_type, "6") == 0) /* ulong */
                dataType = XVariant::UInt;
            else if(strcmp(data_type, "8") == 0) /* string */
                dataType = XVariant::String;
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

                    pinfo("\e[1;4;36mHDB: query %s\e[0m\n", query);

                    res = connection->query(query);
                    if(!res)
                    {
                        snprintf(d_ptr->errorMessage, MAXERRORLEN, "error in query \"%s\": \"%s\"",
                                 query, connection->getError());
                        return false;
                    }

                    notifyEverySteps = round(res->getRowCount() / d_ptr->notifyEveryPercent * totalSources);

                    printf("\e[1;32m notifying every rows %d row cound %d is canceled %d\e[0m\n",
                           notifyEveryPercent, res->getRowCount(), d_ptr->isCancelled);
                    while(!d_ptr->isCancelled && res->next() > 0)
                    {
                        d_ptr->totalRowCnt++;
                        row = res->getCurrentRow();
                        if(!row)
                        {
                            snprintf(d_ptr->errorMessage, MAXERRORLEN, "MysqlHdbSchema.getData: error getting row %d", rowCnt);
                            return false;
                        }

                        if(rowCnt == 0)
                        {
                            fillMode = configHelper->fillFromThePastMode(d_ptr->hdbxSettings,
                                                                         start_date,
                                                                         stop_date,
                                                                         row->getField(0));
                            if(fillMode != ConfigurableDbSchemaHelper::None)
                            {
                                rows_from_the_past = fetchInThePast(source, start_date, table_name, id,
                                                                       dataType, format, wri, connection,
                                                                       &from_the_past_elapsed,
                                                                       fillMode);
                                if(rows_from_the_past >= 0)
                                    rowCnt += rows_from_the_past;
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

//                        printf("\e[1;33m====> every steps %d rows %d total rows %d\e[0m\n",
//                               notifyEverySteps, rowCnt, res->getRowCount());

                        if(notifyEverySteps > 0 && (rowCnt % notifyEverySteps == 0
                                                             || rowCnt == res->getRowCount()) )
                        {
                            d_ptr->resultListenerI->onProgressUpdate(source, (double) rowCnt / res->getRowCount() * myPercent +  myPercent * sourceIndex);
//                            printf("\e[0;33m=====> %f\e[0m\n", (double) rowCnt / res->getRowCount() * myPercent +  myPercent * sourceIndex);
                        }
                    } /* res is closed at the end of else if(wri == XVariant::RW) */

                    success = true;
                }
                else if(wri == XVariant::RW)
                {
                    bool fetchOnlyRead = d_ptr->hdbxSettings && d_ptr->hdbxSettings->getBool("FetchOnlyReadFromRWSource");
                    if(fetchOnlyRead)
                        snprintf(query, MAXQUERYLEN, "SELECT time,read_value FROM %s WHERE time >='%s' "
                                                     " AND time <= '%s' ORDER BY time ASC", table_name, start_date, stop_date);
                    else
                        snprintf(query, MAXQUERYLEN, "SELECT time,read_value,write_value FROM %s WHERE time >='%s' "
                                                 " AND time <= '%s' ORDER BY time ASC", table_name, start_date, stop_date);

                    pinfo("\e[1;4;36mHDB: query %s\e[0m\n", query);

                    res = connection->query(query);
                    if(!res)
                    {
                        snprintf(d_ptr->errorMessage, MAXERRORLEN, "error in query \"%s\": \"%s\"",
                                 query, connection->getError());
                        return false;
                    }

                    notifyEverySteps = round(res->getRowCount() / d_ptr->notifyEveryPercent * totalSources);
                    printf("\e[1;31mrow cnt %d every percent %d total srcs %d\e[0m\n",
                           res->getRowCount(), d_ptr->notifyEveryPercent , totalSources);
                    while(!d_ptr->isCancelled && res->next() > 0)
                    {
                        d_ptr->totalRowCnt++;
                        row = res->getCurrentRow();
                        if(!row)
                        {
                            snprintf(d_ptr->errorMessage, MAXERRORLEN, "MysqlHdbSchema.getData: error getting row %d", rowCnt);
                            return false;
                        }

                        if(rowCnt == 0)
                        {
                            fillMode = configHelper->fillFromThePastMode(d_ptr->hdbxSettings,
                                                                         start_date,
                                                                         stop_date,
                                                                         row->getField(0));
                            if(fillMode != ConfigurableDbSchemaHelper::None)
                            {
                                rows_from_the_past = fetchInThePast(source, start_date, table_name, id,
                                                                       dataType, format, wri, connection,
                                                                       &from_the_past_elapsed,
                                                                       fillMode);
                                if(rows_from_the_past >= 0)
                                    rowCnt += rows_from_the_past;
                            }
                        }

                        rowCnt++;

                        XVariant *xvar = NULL;
                        // pinfo("+ adding %s %s (row count %d)\n", row->getField(0), row->getField(1), res->getRowCount());
                        char *write_data = NULL;
                        if(row->getFieldCount() == 3)
                            write_data = row->getField(2);
                        /* if only the read data was requested, then initialize the XVariant with a NULL write
                         * data
                         */
                        xvar = new XVariant(source, row->getField(0), row->getField(1),
                                            write_data, format, dataType);

                        pthread_mutex_lock(&d_ptr->mutex);
                        if(d_ptr->variantList == NULL)
                            d_ptr->variantList = new XVariantList();
                        d_ptr->variantList->add(xvar);
                        pthread_mutex_unlock(&d_ptr->mutex);

                        row->close();

                        if(notifyEverySteps > 0 && (rowCnt % notifyEverySteps == 0
                                                             || rowCnt == res->getRowCount()) )
                        {
                            double percent = round((double) rowCnt / res->getRowCount() * myPercent  + (myPercent * sourceIndex));
                            printf("\e[1;33m====> %s (RW) every steps %d rows %d total rows %d percent %f\e[0m\n",
                                   source, notifyEverySteps, rowCnt, res->getRowCount(), percent);
                            d_ptr->resultListenerI->onProgressUpdate(source, percent);
                        }
                    } /* end while(res->next() > 0) res is closed at the end */

                    success = (rows_from_the_past >= 0);
                }

                if(!d_ptr->isCancelled && res && res->getRowCount() == 0)
                {
                    pinfo("no rows. Getting from the past");
                    fillMode = configHelper->fillFromThePastMode(d_ptr->hdbxSettings,
                                                                 start_date,
                                                                 stop_date,
                                                                 "");
                    if(fillMode != ConfigurableDbSchemaHelper::None)
                    {

                        rows_from_the_past = fetchInThePast(source, start_date, table_name, id,
                                                               dataType, format, wri, connection,
                                                               &from_the_past_elapsed,
                                                               fillMode);
                        if(rows_from_the_past >= 0)
                            rowCnt += rows_from_the_past;
                    }
                }

                if(res)
                    res->close();


                delete configHelper;

            } /* else: valid data type, format, writable */
        }
        if(d_ptr->isCancelled)
            snprintf(d_ptr->errorMessage, MAXERRORLEN, "MySqlHdbppSchema::mGetData: operation cancelled by the user");
    }
    else
    {
        success = false;
        snprintf(d_ptr->errorMessage, MAXERRORLEN, "MysqlHdbSchema: no attribute \"%s\" in adt", source);
        perr("%s", d_ptr->errorMessage);
    }

    /* compute elapsed time */
    gettimeofday(&tv2, NULL);
    /* transform the elapsed time from a timeval struct to a double whose integer part
     * represents seconds and the decimal microseconds.
     */
    *elapsed = tv2.tv_sec + 1e-6 * tv2.tv_usec - (tv1.tv_sec + 1e-6 * tv1.tv_usec) + from_the_past_elapsed;

    /* notify that the source with name "source" has been extracted */
    d_ptr->resultListenerI->onSourceExtracted(source, rowCnt, *elapsed);

    return !d_ptr->isCancelled && success;
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
                             int notifyEveryPercent)
{
    d_ptr->totalRowCnt = 0;
    double elapsed = 0.0;
    d_ptr->isCancelled = false;
    bool success = getData(source, start_date, stop_date, connection, notifyEveryPercent, 0, 1, &elapsed);
    d_ptr->resultListenerI->onFinished(d_ptr->totalRowCnt, elapsed);
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
                             int notifyEveryPercent)
{
    bool success = true;
    double elapsed = 0.0;
    double perSourceElapsed;
    d_ptr->totalRowCnt = 0;
    d_ptr->isCancelled = false;
    size_t totalSources = sources.size();
    for(size_t i = 0; i < totalSources; i++)
    {
        perSourceElapsed = 0.0;
        printf("MySqlHdbSchema.getData %s %s %s\n", sources.at(i).c_str(), start_date, stop_date);
        success = getData(sources.at(i).c_str(), start_date, stop_date,
                          connection, notifyEveryPercent, i, totalSources, &perSourceElapsed);
        elapsed += perSourceElapsed;
        if(!success)
            printf("\e[1;35m\t\t*\t\t* WARNING: continuing also if success is false!!!\n\t\t*\e[0m\n");
   //     if(!success)
   //         break;
    }

    d_ptr->resultListenerI->onFinished(d_ptr->totalRowCnt, elapsed);
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

    pinfo("\e[1;34mQUERY %s\e[0m\n", query);
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

/** \brief Looks in the database for data before start_date applying to source source
 *
 * @return -1 if an error occurs
 * @return a positive number representing the number of rows extracted
 */
int MySqlHdbSchema::fetchInThePast(const char *source,
                                    const char *start_date, const char *table_name,
                                    const int /* att_id */,
                                    XVariant::DataType dataType,
                                    XVariant::DataFormat format,
                                    XVariant::Writable writable,
                                    Connection *connection,
                                    double *time_elapsed,
                                    ConfigurableDbSchemaHelper::FillFromThePastMode mode)
{
    int ret = -1;
    char query[MAXQUERYLEN];
    char timestamp[MAXTIMESTAMPLEN];
    struct timeval tv1, tv2;
    Result *res = NULL;
    Row *row = NULL;
    bool fetchOnlyRead = d_ptr->hdbxSettings &&
            d_ptr->hdbxSettings->getBool("FetchOnlyReadFromRWSource");

    gettimeofday(&tv1, NULL);

    pinfo("HDB: fetching in the past \"%s\" before %s", source, start_date);
    if(writable != XVariant::RW && !fetchOnlyRead)
    {
        snprintf(query, MAXQUERYLEN, "SELECT time,value FROM "
                                     " %s WHERE time <= '%s' "
                                     " ORDER BY time DESC LIMIT 1",
                 table_name, start_date);
    }
    else if(writable == XVariant::RW && fetchOnlyRead)
    {
        snprintf(query, MAXQUERYLEN, "SELECT time,read_value FROM "
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

    pinfo("HDB: query: %s", query);
    res = connection->query(query);
    if(!res)
    {
        snprintf(d_ptr->errorMessage, MAXERRORLEN, "MySqlHdbSchema.fetchInThePast: bad query \"%s\": \"%s\"",
                 query, connection->getError());
        return -1;
    }

    ret = res->getRowCount();

    while(res->next() > 0)
    {
        row = res->getCurrentRow();

        if(!row)
        {
            snprintf(d_ptr->errorMessage, MAXERRORLEN, "MySqlHdbSchema.fetchInThePast: error getting row");
            return -1;
        }
        else
        {
            XVariant *xvar = NULL;
            /* choose timestamp according to ConfigurableDbSchemaHelper mode */
            printf("\e[1;35mchoosing form %s and %s mode %d\e[0m\n", start_date, row->getField(0), mode);
            if(mode == ConfigurableDbSchemaHelper::KeepWindow)
                strncpy(timestamp, start_date, MAXTIMESTAMPLEN);
            else
                strncpy(timestamp, row->getField(0), MAXTIMESTAMPLEN);

            printf("\e[1;35mchosen form %s \e[0m\n", timestamp);

            /* if fetchOnlyRead is true, then the result will contain the read part only
             * and the write data will be null.
             */
            if(writable != XVariant::RW)
                xvar = new XVariant(source, timestamp, row->getField(1), format, dataType, writable);
            else if(writable == XVariant::RW && row->getFieldCount() == 2)
                xvar = new XVariant(source, timestamp, row->getField(1), NULL, format, dataType);
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
    return ret;
}

/** \brief Interrupts the execution of the next query. An error message will be set and
 *         the success of the operation is set to false
 */
void MySqlHdbSchema::cancel()
{
    d_ptr->isCancelled = true;
}

bool MySqlHdbSchema::isCancelled() const
{
    bool is;
    pthread_mutex_lock(&d_ptr->mutex);
    is = d_ptr->isCancelled;
    pthread_mutex_unlock(&d_ptr->mutex);
    return is;
}

/** \brief Manually reset the cancelled flag.
 *
 * The getData method with the additional parameters  sourceIndex and
 * totalSources  (which is intended for special use) does not reset the
 * cancelled flag. If you happen to use that version of getData, be sure to
 * call resetCancelledFlag before.
 *
 */
void MySqlHdbSchema::resetCancelledFlag() const
{
    d_ptr->isCancelled = false;
}



