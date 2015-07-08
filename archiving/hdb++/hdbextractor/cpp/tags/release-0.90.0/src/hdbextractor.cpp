#include "hdbextractor.h"
#include "hdbextractorprivate.h"
#include "mysqlconnection.h"
#include "result.h"
#include "hdbxmacros.h"
#include "hdb/src/mysqlhdbschema.h"
#include "hdbpp/src/mysqlhdbppschema.h"
#include "hdbextractorlistener.h"
#include "queryconfiguration.h"
#include "timeinterval.h"

#include <string.h>


Hdbextractor::~Hdbextractor()
{
    if(d_ptr->connection != NULL)
    {
        pinfo("~HdbExtractor: closing db connection");
        d_ptr->connection->close();
    }
}

Hdbextractor::DbType Hdbextractor::dbType() const
{
    return d_ptr->dbType;
}

void Hdbextractor::setDbType(DbType dbt)
{
    if(d_ptr->dbschema != NULL)
        delete d_ptr->dbschema;

    d_ptr->dbType = dbt;
    switch(dbt)
    {
    case HDBMYSQL:
        d_ptr->dbschema = new MySqlHdbSchema(this);
        break;
    case HDBPPMYSQL:
        d_ptr->dbschema = new MySqlHdbppSchema(this);
        break;
    case DBUNDEFINED:
        pfatal("HdbExtractor.setDbType: db type undefined. Please call setDbType");
        break;
    default:
        pfatal("HdbExtractor.setDbType: db type %d unsupported", dbt);
        break;
    }
}


/** \brief The Hdbextractor++ historical database data extractor. Main class.
 *
 * <h3>Introduction</h3>
 *  The Hdbextractor++ framework allows fetching data from an historical database (hdb, hdb++, influxDB).
 *  The access to the data retrieved from one of the supported
 *  databases is thread safe.
 *
 * <h3>Usage</h3>
 *  The use of the Hdbextractor is very simple.
 *  The class that makes use of the Hdbextractor must implement the HdbExtractorListener interface in order
 *  to be notified when the data has been partially or completely fetched from the database.
 *  This is compulsory either if you are not using Hdbextractor in a multi-threaded environment or you are not
 *  actually interested in splitting the results into blocks of data as soon as a new bulk has been fetched.
 *  A single interface for accessing data is defined. This choice makes things clearer and straight.
 *
 *  Upon implementing the HdbExtractorListener interface you will write the following methods:
 *  <ul>
 *  <li>void HdbExtractorListener::onProgressUpdate(int step, int total);</li>
 *  <li>void HdbExtractorListener::onFinished(int totalRows); </li>
 *  </ul>
 *
 *  Calling setUpdateProgressStep with an integer value greater than 0 determines whether onProgressUpdate
 *  is called or not in your HdbExtractorListener implementation. By default, it is not called
 *  (the method updateProgressStep returns -1) and
 *  you will be notified that the fetch has been completed when your implementation of the
 *  onFinished method is invoked.
 *  At that time, you can retrieve the actual data as a whole by calling get.
 *
 *  When calling get, you have to supply a reference to a std::vector<XVariant>.
 *  Data is always appended to your vector, and the vectory you provide will never be cleared before
 *  data is pushed on its back.
 *  This allows to accumulate data on the destination vector upon partial data fetch progress
 *  (through onProgressUpdate)
 *
 *  \note The HdbExtractorListener::onProgressUpdate is called according to the number configured with
 *  Hdbextractor::setUpdateProgressStep but also when the last bulk of data is available, even if its number of
 *  rows is less than the configured value.
 *
 *  Obtaining data with the Hdbextractor::get method is <em>thread safe</em>.
 *  If you opt for partial data fetching, you will invoke Hdbextractor::get within <em>your implementation of </em>
 *  onProgressUpdate. Every time it is called,
 *  the partial data is copied over to the std::vector you supply as parameter of get.
 *  Afterwards, the source of the data is erased from the
 *  Hdbextractor, the memory is freed and so on. Subsequent calls to Hdbextractor::get will never provide data which has already been
 *  obtained with previous Hdbextractor::get invocations.
 *  On the other hand, <em>you will not have to worry about data deletion at any point in the
 *  extraction</em> and this <em>will be true for every data structure used in this library</em>. We deem that not
 *  having the responsibility of data objects' memory handling is a big comfort for the end user of this library.
 */
Hdbextractor::Hdbextractor(HdbExtractorListener *hdbxlistener) : ResultListener()
{
    d_ptr = new HdbExtractorPrivate();
    d_ptr->connection = NULL;
    d_ptr->dbschema = NULL;
    d_ptr->dbType = DBUNDEFINED;
    d_ptr->hdbXListenerI = hdbxlistener;
    d_ptr->queryConfiguration = NULL;
    /* by default, trigger data available on listener only when data fetch is complete */
    d_ptr->updateEveryRows = -1;
}

/** \brief This method is used to create a network connection with the database.
 *
 */
bool Hdbextractor::connect(DbType dbType, const char *host,
                           const char *db,
                           const char *user,
                           const char *passwd,
                           unsigned short port)
{
    bool success = false;
    strcpy(d_ptr->errorMessage, ""); /* clear error message */

    switch(dbType)
    {
    case HDBMYSQL:
    case HDBPPMYSQL:
        d_ptr->connection = new MySqlConnection();
        success = d_ptr->connection->connect(host, db, user, passwd, port);
        pinfo("HdbExtractor: connected: %d host %s  db %s user %s passwd %s", isConnected(), host, db, user, passwd);
        break;
    default:
        pfatal("HdbExtractor: connect: database type unsupported: %d", dbType);
        break;
    }

    /* set the desired schema and initialize DbSchema */
    setDbType(dbType);

    if(!success)
        strncpy(d_ptr->errorMessage, d_ptr->connection->getError(), MAXERRORLEN);

    return success;
}

void Hdbextractor::disconnect()
{
    if(d_ptr->connection != NULL && d_ptr->connection->isConnected())
        d_ptr->connection->close();
}

/** \brief Returns the <em>last error</em> message referring to the last encountered problem
 *
 * @return a string representing the error message.
 *
 * You can test whether there are errors or not by calling hasError first
 *
 * @see hasError
 *
 */
const char*  Hdbextractor::getErrorMessage() const
{
    return d_ptr->errorMessage;
}

/** \brief Returns an error flag revealing if the last operation was successful or not
 *
 * @return true the last operation failed
 *
 */
bool  Hdbextractor::hasError() const
{
    return (strlen(d_ptr->errorMessage) > 0);
}

/** \brief Start fetching data from the database. When data is available, you can get it
 *         with the HdbExtractor::get method inside onFinished or onProgressUpdate, if
 *         partial data updates are preferred.
 *
 * @param source the name of a tango device attribute, full name, e.g. domain/family/member/attName
 * @param start_date the start date in the form "2014-07-10 10:00:04"
 * @param stop_date  the stop date in the form "2014-07-10 10:20:04"
 *
 * @see XVariant
 * @see get
 * @see setUpdateProgressStep
 *
 * @return true if the data fetch was successful, false otherwise.
 *
 * \par Query options.
 * See setQueryConfiguration and the QueryConfiguration object to see what options can be
 * applied to the database queries. For example, it is possible to choose the desired behaviour
 * when no data is available between start_date and stop_date.
 *
 * If this call was not successful, you can call getErrorMessage to get the error message
 *
 * @see getErrorMessage
 * @see setQueryConfiguration
 *
 */
bool Hdbextractor::getData(const char *source,
                           const char *start_date,
                           const char *stop_date)
{
    bool success = false;
    strcpy(d_ptr->errorMessage, "");
    printf("HdbExtractor.getData %s %s %s\n", source, start_date, stop_date);
    if(d_ptr->connection != NULL && d_ptr->dbschema != NULL && d_ptr->connection->isConnected())
    {
        if(d_ptr->queryConfiguration != NULL)
            d_ptr->dbschema->setQueryConfiguration(d_ptr->queryConfiguration);
        success = d_ptr->dbschema->getData(source, start_date, stop_date,
                                           d_ptr->connection, d_ptr->updateEveryRows);
    }
    /* error message, if necessary */
    if(!success)
        snprintf(d_ptr->errorMessage, MAXERRORLEN, "Hdbextractor.getData: %s", d_ptr->dbschema->getError());

    return success;
}

/** \brief This is a convenience function that calls the single source getData for each
 *         source provided in the list.
 *
 * @param sources a vector of chars each one is the name of the attribute
 *
 * @return true if the data fetch was successful, false otherwise.
 *
 * If this call was not successful, you can call getErrorMessage to get the error message
 *
 * \par Query options.
 * See setQueryConfiguration and the QueryConfiguration object to see what options can be
 * applied to the database queries. For example, it is possible to choose the desired behaviour
 * when no data is available between start_date and stop_date.
 *
 * @see getErrorMessage
 * @see setQueryConfiguration
 */
bool Hdbextractor::getData(const std::vector<std::string> sources,
                           const char *start_date,
                           const char *stop_date)
{
    bool success = false;
    strcpy(d_ptr->errorMessage, "");

    if(d_ptr->connection != NULL && d_ptr->dbschema != NULL && d_ptr->connection->isConnected())
    {
        if(d_ptr->queryConfiguration != NULL)
            d_ptr->dbschema->setQueryConfiguration(d_ptr->queryConfiguration);

        for(size_t i = 0; i < sources.size(); i++)
        {
            printf("HdbExtractor.getData %s %s %s\n", sources.at(i).c_str(), start_date, stop_date);
            success = d_ptr->dbschema->getData(sources.at(i).c_str(), start_date, stop_date,
                                               d_ptr->connection, d_ptr->updateEveryRows);
            if(!success)
                break;
        }
    }
    /* error message, if necessary */
    if(!success)
        snprintf(d_ptr->errorMessage, MAXERRORLEN, "Hdbextractor.getData: %s", d_ptr->dbschema->getError());

    return success;
}

bool Hdbextractor::getData(const char *source, const TimeInterval *time_interval)
{
    return getData(source, time_interval->start(), time_interval->stop());
}

bool Hdbextractor::getData(const std::vector<std::string> sources, const TimeInterval *time_interval)
{
    return getData(sources, time_interval->start(), time_interval->stop());
}

bool Hdbextractor::getSourcesList(std::list<std::string>& result) const
{
    bool success = false;
    strcpy(d_ptr->errorMessage, "");
    printf("HdbExtractor.getSourcesList\n");
    if(d_ptr->connection != NULL && d_ptr->dbschema != NULL && d_ptr->connection->isConnected())
        success = d_ptr->dbschema->getSourcesList(d_ptr->connection, result);

    /* error message, if necessary */
    if(!success && d_ptr->dbschema)
        snprintf(d_ptr->errorMessage, MAXERRORLEN, "Hdbextractor.getSourcesList: %s", d_ptr->dbschema->getError());

    return success;
}

bool Hdbextractor::findErrors(const char *source, const TimeInterval *time_interval) const
{
    bool success = false;
    strcpy(d_ptr->errorMessage, "");
    if(d_ptr->connection != NULL && d_ptr->dbschema != NULL && d_ptr->connection->isConnected())
        success = d_ptr->dbschema->findErrors(source, time_interval, d_ptr->connection);
    /* error message, if necessary */
    if(!success && d_ptr->dbschema)
        snprintf(d_ptr->errorMessage, MAXERRORLEN, "Hdbextractor.findErrors: %s", d_ptr->dbschema->getError());
    return success;
}

/** \brief Append the available data into the variantlist passed as parameter
 *
 * This function copies the available data (which may be partial or complete,
 * depending at which point this method is called) into variantlist.
 * Data is appended to the variantlist passed as parameter.
 *
 * \note Connection must obviously be setup and the database chosen.
 *
 * If no data is available or the database is not correctly setup, the variantlist
 * is left untouched
 */
int Hdbextractor::get(std::vector<XVariant>& variantlist)
{
    if(d_ptr->dbschema != NULL)
        return d_ptr->dbschema->get(variantlist);
    return -1;
}

bool Hdbextractor::isConnected() const
{
    return d_ptr->connection != NULL;
}

/** \brief Returns the number of rows after which a progress update must be triggered on the listener.
 *
 * @return the number of extracted rows determining the frequency of the onProgressUpdate invocation
 *
 * @see onProgressUpdate
 * @see setUpdateProgressStep to set the current step value
 */
int Hdbextractor::updateProgressStep()
{
    return d_ptr->updateEveryRows;
}

/** \brief This method allows to configure various options before querying the database
 *
 * @param qc The QueryConfiguration object with the desired options set.
 *
 * This method is used to configure the way data is fetched by getData.
 *
 * @see QueryConfiguration
 * @see getData
 */
void Hdbextractor::setQueryConfiguration(QueryConfiguration *qc)
{
    d_ptr->queryConfiguration = qc;
}

/** \brief set the number of rows after which a progress update must be triggered on the listener.
 *
 * @param numRows every numRows onProgressUpdate is invoked
 *
 * @see onProgressUpdate
 * @see updateProgressStep to get the current step value
 */
void Hdbextractor::setUpdateProgressStep(int numRows)
{
    d_ptr->updateEveryRows = numRows;
}

/** \brief Implements ResultListener::onProgressUpdate interface
 *
 */
void Hdbextractor::onProgressUpdate(const char *name, int step, int total)
{
    d_ptr->hdbXListenerI->onSourceProgressUpdate(name, step, total);
}

/** \brief Implements ResultListener::onFinished interface
 *
 */
void Hdbextractor::onFinished(const char *name, int sourceStep, int totalSources, double elapsed)
{
    d_ptr->hdbXListenerI->onSourceExtracted(name, sourceStep, totalSources, elapsed);
}


