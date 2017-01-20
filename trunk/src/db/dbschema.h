#ifndef DBSCHEMA_H
#define DBSCHEMA_H

#include <vector>
#include <list>
#include <string>

class Connection;
class XVariantList;
class XVariant;
class XErrorData;
class HdbXSettings;
class TimeInterval;

/** \brief The interface representing a database schema. <em>Used internally</em>.
 *
 * \interface DbSchema
 *
 * This interface provides the main method to retrieve data from the database.
 * Any implementation of this interface is specific to a database (e.g. MySql, InfluxDB)
 * and a schema (hdb, hdb++, ...)
 *
 */
class DbSchema
{
public:
    DbSchema() {}

    virtual ~DbSchema() {}

    /** \brief Fetch data from the database.
     *
     * Fetch data from the database.
     * \note This method is used by HdbExtractor and it is not meant to be directly used by the library user.
     *
     * @param source the tango attribute in the form domain/family/member/AttributeName
     * @param start_date the start date (begin of the requested data interval) as string, such as "2014-07-10 10:00:00"
     * @param stop_date the stop date (end of the requested data interval) as string, such as "2014-07-10 12:00:00"
     * @param connection the database Connection specific object
     * @param notifyEveryRows the number of rows that make up a block of data. Every time a block of data is complete
     *        notifications are sent to the listener of type ResultListener (HdbExtractor)
     *
     *
     * @return true if the call was successful, false otherwise.
     */
    virtual bool getData(const char *source,
                                    const char *start_date,
                                    const char *stop_date,
                                    Connection *connection,
                                    int notifyEveryRows) = 0;

    virtual bool getData(const std::vector<std::string> sources,
                                 const char *start_date,
                                 const char *stop_date,
                                 Connection *connection,
                                 int notifyEveryNumRows) = 0;

    virtual bool getData(const char *source,
                                    const TimeInterval *time_interval,
                                    Connection *connection,
                                    int notifyEveryRows) = 0;

    virtual bool getData(const std::vector<std::string> sources,
                                    const TimeInterval *time_interval,
                                    Connection *connection,
                                    int notifyEveryRows) = 0;

    /** \brief This method allows getting data for the given source. The convenience parameters
     *         sourceIndex and totalSources help you getting multiple sources separately.
     *
     * Normally one should rely on the other methods for the retrieval of multiple sources, but if
     * you want to perform some operation between two subsequent data extractions, then this is the
     * way to go. The sourceIndex and totalSources parameters must be spefified in order to get
     * the correct progress updates on the extraction listeners.
     *
     * @param source the name of the source
     * @param start_date the start date (begin of the requested data interval) as string, such as "2014-07-10 10:00:00"
     * @param stop_date the stop date (end of the requested data interval) as string, such as "2014-07-10 12:00:00"
     * @param connection the database Connection specific object
     * @param notifyEveryRows the number of rows that make up a block of data. Every time a block of data is complete
     *        notifications are sent to the listener of type ResultListener (HdbExtractor)
     * @param sourceIndex the index of the current source (starting from 0 and ending to sources size - 1
     * @param totalSources the total number of sources to extract.
     *
     * This method is invoked by the other multiple source getData in this library. This may serve you as
     * an example (see mysqlhdbschema.cpp and mysqlhdbppschema.cpp).
     *
     */
    virtual bool getData(const char *source,
                  const char *start_date,
                  const char *stop_date,
                  Connection *connection,
                  int notifyEveryPercent,
                  int sourceIndex,
                  int totalSources, double *elapsed) = 0;

    /** \brief Retrieves the list of archived sources, returning true if the query is successful.
     *
     * Retrieve the list of sources archived into the database, sorted alphabetically from a to z.
     *
     * @param result a std::list of std::string that will store the result
     *        of the search.
     *
     * @return false if a database error occurred, true otherwise.
     *
     * If false is returned, an error occurred: you can get the message through getError.
     */
    virtual bool getSourcesList(Connection *connection, std::list<std::string>& result) const = 0;

    /** \brief Finds a source containing the provided substring.
     *
     * @param substring the search string
     * @param connection the database connection
     * @param result a std::list of std::string that will store the result
     *        of the search.
     *
     * @return the number of sources matching the substring provided.
     *
     * If false is returned, an error occurred: you can get the message through getError.
     *
     * \note For MySql databases, the <em>like</em> keyword is used, with substring as argument.
     */
    virtual bool findSource(Connection *connection, const char *substring, std::list<std::string>& ) const = 0;

    /** \brief Empties the queue where partial (or complete) data is stored
     *
     * \note This function is thread safe
     *
     * @return the number of XVariant elements appended to variantlist, -1 in case an error happened somewhere
     */
    virtual int get(std::vector<XVariant>& variantlist) = 0;

    /** \brief This method finds the errors occurred inside a time_interval window for the specified source
     *
     * @param source The name of the source to look for in the database
     * @param time_interval a TimeInterval defining the time window of interest
     * @param connection the database connection to use
     *
     * The results can be obtained with the method get. The ResultListenerInterface::onProgressUpdate
     * and ResultListenerInterface::onFinished can be used in order to receive notifications.
     *
     * @return true if the query to the database is successfull, false otherwise.
     *
     * \note The XVariant data structure is used to store the results, even if it is overabundant.
     * Each XVariant will store the quality factor, the error message and the associated timestamp.
     * variantlist will be filled with data which either has an <em>invalid quality factor</em>,
     * or an error description which is not null.
     * If no errors occurred in the specified time interval, variantlist will be left unchanged.
     *
     * \note findErrors will not fetch the data value: rows with a null data value (read or write)
     * are not selected by this method. findErrors evaluates only the quality factor and a <em>non
     * empty</em> error string. Moreover, only an <em>invalid quality factor (ATTR_INVALID in Tango)
     * </em> is considered to be an error marker. The Tango ATTR_INVALID quality corresponds to the
     * value 1, as you can see from idl/tango.h include file:
     * enum AttrQuality { ATTR_VALID, ATTR_INVALID, ATTR_ALARM, ATTR_CHANGING,
     * ATTR_WARNING, __max_AttrQuality=0xffffffff };
     *
     * \note The hdb database does not support errors.
     */
    virtual bool findErrors(const char *source, const TimeInterval *time_interval,
                            Connection *connection) const = 0;

    /** \brief This method returns the last error message that occurred.
     *
     * If during the last operation no error occurred, this method should return an empty string
     * and hasError should return false.
     * The return value is not empty only if the last operation caused an error. This means that
     * previous error messages are lost.
     *
     * @return a string representing the error occurred during the last operation.
     *
     * @see hasError
     */
    virtual const char *getError() const = 0;

    /** Returns whether the last operation lead to an error or not
     *
     * @see getError
     */
    virtual bool hasError() const = 0;

    virtual void cancel() = 0;

    virtual bool isCancelled() const = 0;

    /** \brief Manually set to false the cancelled flag.
     *
     * This should not be needed unless special use of the getData version with
     * the sourceIndex and the totalSources parameters is used.
     *
     * Please read the specific implementation of this method in subclasses.
     *
     * @see MySqlHdbSchema::resetCancelledFlag
     * @see MySqlHdbppSchema::resetCancelledFlag
     */
    virtual void resetCancelledFlag() const = 0;

};

#endif // DBSCHEMA_H
