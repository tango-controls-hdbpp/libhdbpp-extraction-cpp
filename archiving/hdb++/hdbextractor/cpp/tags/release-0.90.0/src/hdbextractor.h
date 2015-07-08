#ifndef HDBEXTRACTOR_H
#define HDBEXTRACTOR_H

#include <xvariantlist.h>
#include <vector>
#include <string>
#include <list>

#include "resultlistenerinterface.h"

class HdbExtractorPrivate;
class HdbExtractorListener;
class QueryConfiguration;
class TimeInterval;

/** \mainpage The Hdbextractor++ historical database data extractor
 *
 * <h3>Introduction</h3>
 *  The Hdbextractor++ framework allows fetching data from an historical database (hdb, hdb++, influxDB)
 *  in a simple, fast object oriented fashion. The access to the data retrieved from one of the supported
 *  databases is thread safe.
 *
 * \par Qt module
 *  Please read the QHdbExtractor documentation manual to learn how to use the Qt module.
 *  The Qt module sources are located under the qt/hdbextractor-qt subfolder.
 *  A ready to use Qt graphical user interface is available under the qt/QHdbExtractor subfolder.
 *
 *
 * \par Scalar data visualization.
 * \image html qhdbextractor.png
 *
 *
 * \par Error visualization.
 * Red vertical lines represent a NULL value into the database at the corresponding date time.
 * A click close to the invalid data point opens a right pane where error details are available.
 * Please read the QHdbExtractor documentation manual to learn how to use the Qt module.
 * These screenshots are taken from the Qt graphical user interface that you can find under the
 * qt/QHdbExtractor subfolder.
 *
 * \image html qhdbX-errors.png
 *
 * \par Important note
 * Even if the HdbExtractor library is tailored to deal with Tango data (as far as data type,
 * writable property and data format are concerned), it is <em>not dependent</em> from Tango
 * specific data types and libraries at any rate.
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
 *
 *  Let's briefly discuss what this involves analyzing a use case. Suppose that the the library supplying plot widgets
 *  provides a method called appendData(string curveName, const vector<double>& timestamps, const vector<double>& data).
 *  This method appends x and y data to the curve with a given curveName. Data is read from the database in the background
 *  by a separate thread. The Hdbextractor has been configured to notify the availability of new data whenever 100 rows
 *  have been fetched from the server. See what happens inside your void HdbExtractorListener::onProgressUpdate
 *  implementation.
 *
 *  \code
// New data is available
void MyHdbExtractorImplementation::onProgressUpdate(int step, int total)
{
    // valuelist will host the available data
    std::vector<XVariant> valuelist;

    // get the data in a thread-safe fashion
    // the data fetched from the database is moved into valuelist.
    // This data will not be available the next time get is invoked.
    // valuelist lifetime is limited to the scope of this method.
    // Associated memory on the Hdbextractor side is automatically managed.
    // mExtractor is a reference to a previously created and configured Hdbextractor
    mExtractor->get(valuelist);

    // vectors to store data to be passed to the plot for drawing
    std::vector<double> timestamps, data;

    // fill them in
    for(size_t i = 0; i < valuelist.size(); i++)
    {
        XVariant v = valuelist[i];
        // suppose we have written a toMilliseconds(const char *timestampString)
        // to convert the timestamp to a timestamp in milliseconds
        timestamps.push_back(v.getTime_tTimestamp());
        data.push_back(v.toDouble());
    }

    // append data to the plot.
    //
    // Note that the methods toDouble(), toDoubleVector() and so on return a copy
    // of the stored data, so that the memory used by them is different from the
    // memory used by the Hdbextractor data.
    //
    // Hdbextractor data memory is freed after get is invoked.
    // valuelist dies at the end of this method, because it's local, but timestamps
    // and data are taken over by the plot.
    myPlot->appendData(v.getSource(), timestamps, data);
}

  \endcode

 *
 * \par Example
 *
 * Let's now provide a full working sample code of a single threaded application.
 * The program receives a tango source point, a start date and a stop date from the command line.
 * The source point is the full name of a tango attribute: domain/family/member/attributeName
 * Double scalar or vector data is printed on the screen.
 *
 * We start defining a class called MyHdbExtractorImpl that implements the HdbExtractorListener
 * interface.
 *
 * \code

#include <hdbextractorlistener.h>

class Hdbextractor;

class MyHdbExtractorImpl : public HdbExtractorListener
{
public:
    MyHdbExtractorImpl();

    // fetch data on the database
    void getData(const char* source, const char* start_date, const char *stop_date);

    // our implementation
    virtual void onProgressUpdate(int step, int total);

    // our implementation
    virtual void onFinished(int totalRows);

private:
    Hdbextractor *mExtractor;
};

 \endcode
 *
 * Here comes the cpp part of MyHdbExtractorImpl
 *
 * \code

MyHdbExtractorImpl::MyHdbExtractorImpl()
{
    const char *dbuser = "hdbbrowser";
    const char *dbpass = "hdbbrowser";
    const char *dbhost = "fcsproxy";
    const char *dbnam = "hdb";

    // Let MyHdbExtractorImpl take care of an instance of Hdbextractor.
    //
    mExtractor = new Hdbextractor(this);
    try{
        mExtractor->connect(Hdbextractor::HDBMYSQL, dbhost, dbnam, dbuser, dbpass);
        mExtractor->setUpdateProgressStep(100);
    }
    catch(const HdbXException &e)
    {
        printf("error connecting to host: %s\n", e.getMessage());
    }

}

void MyHdbExtractorImpl::getData(const char* source, const char* start_date, const char *stop_date)
{
    try{
        mExtractor->getData(source, start_date, stop_date);
    }
    catch(const HdbXException &e)
    {
        printf("error fetching data: %s\n", e.getMessage());
    }
}

// You can see the snippet above for comments
// This version of onProgressUpdate provides print routines for scalar and
// vector data formats
//
void MyHdbExtractorImpl::onProgressUpdate(int step, int total)
{
    std::vector<XVariant> valuelist;
    // get the buffered values accumulated between the previous call to
    // get and this one. get() is thread safe.
    mExtractor->get(valuelist);

    for(size_t i = 0; i < valuelist.size(); i++)
    {
        XVariant::DataFormat format = valuelist[i].getFormat();
        if(format == XVariant::Scalar)
        {
            printf("%.2f, ", valuelist[i].toDouble());
            if(i > 0 && i % 20 == 0)
                printf("\n");
        }
        else if(format == XVariant::Vector)
        {
            std::vector<double> values = valuelist[i].toDoubleVector();
            printf("[ ");
            for(size_t j = 0; j < values.size(); j++)
                printf("%g ,", values[j]);
            printf(" ]\n");
        }
    }
    printf("\n");
}

void MyHdbExtractorImpl::onFinished(int totalRows)
{
    printf("extraction completed: got %d rows\n", totalRows);
}

  \endcode
 *
 * Now we examine the main.cpp file. What we have to do is allocate an instance of our
 * MyHdbExtractorImpl and invoke getData when we want to trigger the data retrieval.
 *
 * \code
#include <stdio.h>
#include <stdlib.h>
#include "myhdbextractorimpl.h"
#include <hdbextractor.h>

using namespace std;

int main(int argc, char **argv)
{
    // Check the command line arguments
    if(argc < 4)
    {
        printf("Usage\e[0m \"%s domain/family/member/attribute 2014-07-20 10:00:00 2014-07-20 12:00:00\n",
               argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        // simply create an instance of our MyHdbExtractorImpl
        // implementation
        //
        MyHdbExtractorImpl *hdbxi = new MyHdbExtractorImpl();
        // ...and call get data with the source name and the start and stop time
        //
        hdbxi->getData(argv[1], argv[2], argv[3]);
    }
    return 0;
}

  \endcode
 *
 *
 * @see HdbExtractorListener
 * @see XVariant
 * @see QHdbextractorProxy (Qt module)
 * @see QHdbExtractor (Qt application example)
 *
 *
 * \par Manage your own configuration files.
 * The ConfigurationParser class can be used to store couples key/value in form of
 * strings. The ConfigurationParser reads a text file to load the settings. The
 * file is a plain text one, whose lines are of the kind
 *
  \code
  # Comments start after a # character
  #
  # Empty lines are allowed:

  #
  mykey = myvalue # inline comments
  # Each key / value couple is treated as string by the ConfigurationParser
  mynumeric_key = 10.0

  \endcode
 *
 * The ConfigurationParser can be used to store the connection parameters.
 *
 * @see ConfigurationParser
 *
 * \par Customize queries and data fetch
 * Through the QueryConfiguration class it is possible to customize queries according to
 * specific needs. For example, if no data is stored in a given period of time, QueryConfiguration
 * can be told to retrieve the first valid data from the past or to simply return an empty result.
 * QueryConfiguration stores string key/value couples, and converts it to booleans, integers or
 * doubles if requested. QueryConfiguration can load settings from a file, with the same format as
 * ConfigurationParser's. Moreover, the same configuration file can be shared by QueryConfiguration
 * and your own ConfigurationParser.
 * The ConfigurationParser can be used to store the connection parameters.
 *
 * \par Sieving results of multiple source queries and aligning data in time.
 * The DataSiever utility class can be used to separate the data of each different source (attribute)
 * given the vector of XVariant returned by Hdbextractor::getData.
 * Once organized by source name, each vector of data can be aligned in time, with the DataSiever::fill
 * method. See the DataSiever documentation for further details.
 *
 * @see DataSiever
 */
class Hdbextractor :  ResultListener
{


public:

    /** \brief the database type to use. This enum lists the supported combinations of database types
     *         and schemas.
     *
     * Currently supported databases:
     * \li Hdb with mysql
     *
     */
    enum DbType { DBUNDEFINED = -1, HDBMYSQL, HDBPOSTGRES, HDBPPMYSQL, HDBPPINFLUX, HDBPPPOSTGRES };

    Hdbextractor(HdbExtractorListener *hdbxlistener);

    virtual ~Hdbextractor();

    DbType dbType() const;

    virtual void setUpdateProgressStep(int numRows);

    /** \brief returns the number of rows after which onProgressUpdate is called on the registered HdbExtractorListener
     *
     * @return the number of rows after which onProgressUpdate is called on the registered HdbExtractorListener
     *
     * @see setUpdateProgressStep
     * @see HdbExtractorListener
     * @see getData
     */
    int updateProgressStep();

    /** \brief Try to establish a database connection with the specified host, user, password and database name
     *
     * @param dbType A type of database as defined in the DbType enum
     * @param host the internet host name of the database server
     * @param user the user name of the database
     * @param passwd the password for that username
     * @param port the database server port (default 3306, thought for mysql)
     *
     * Throws an exception upon failure.
     */
    bool connect(DbType dbType, const char* host, const char *db, const char* user,
                 const char* passwd, unsigned short port = 3306);

    /** \brief Disconnect the client from the database
     *
     * This call can be used to explicitly disconnect the client from the database.
     */
    void disconnect();

    bool getData(const char* source, const char* start_date, const char *stop_date);

    bool getData(const std::vector<std::string> sources,
                                          const char *start_date,
                                          const char *stop_date);

    bool getData(const char *source, const TimeInterval *time_interval);

    bool getData(const std::vector<std::string> sources, const TimeInterval *time_interval);

    bool getSourcesList(std::list<std::string>& result) const;

    bool findErrors(const char *source, const TimeInterval *time_interval) const;

    void setQueryConfiguration(QueryConfiguration *qc);

    /** \brief Get a copy of the partial or complete data fetched from the database up to this moment
     *
     * The data already fetched is copied into a list of XVariant. A reference to a vector is passed.
     * The data is no more available after it has been fetched with get.
     *
     * @see XVariant
     * @see getData
     */
    int get(std::vector<XVariant>& variantlist);

    bool isConnected() const;

    void setDbType(DbType dbt);

    virtual void onProgressUpdate(const char* name, int step, int total);

    virtual void onFinished(const char *name, int sourceStep, int totalSources, double elapsed);

    const char* getErrorMessage() const;

    bool hasError() const;

private:
    HdbExtractorPrivate *d_ptr;
};

#endif // HDBEXTRACTOR_H
