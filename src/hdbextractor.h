#ifndef HDBEXTRACTOR_H
#define HDBEXTRACTOR_H

#include <xvariantlist.h>
#include <resultlistenerinterface.h>

#include <vector>
#include <string>
#include <list>

class HdbExtractorPrivate;
class HdbExtractorListener;
class HdbXSettings;
class TimeInterval;

/** \mainpage The Hdbextractor++ historical database data extractor
 *
 * <h2>Introduction</h2>
 *  The Hdbextractor++ framework allows fetching data from an historical database (hdb, hdb++)
 *  in a simple object oriented fashion. The access to the data retrieved from one of the supported
 *  databases is thread safe.
 *
 * \par Qt module
 * Please read the hdbextractor-qt documentation manual to study the Qt module.
 * The Qt module sources are located under  a separate project named hdbextractor-qt.
 * A ready to use Qt graphical user interface is available under the hdbextractor-qt/QHdbExtractor subfolder.
 * The QHdbExtractor application deals with displaying scalar and vector data in 2D and 3D plots
 * respectively.
 *
 * \par Scalar data visualization.
 *
 * \image html qhdbextractor.png
 *
 * \image latex qhdbextractor.png
 *
 *
 * \par Vector data visualization.
 * Vectors can be displayed over time using a surface plot. The QHdbExtractor application combines
 * the hdbextractor, the hdbextractor-qt and the mathgl/qdddplot libraries to draw surface plots.
 * Mathgl is a powerful data visualization library. It can be downloaded from svn with the following command:
 *
 * \code
 * svn checkout http://svn.code.sf.net/p/mathgl/code/mathgl-2x mathgl-code
 * \endcode
 *
 * The Qdddplot library wraps the mathgl code to allow an easy integration with Qt and in particular with
 * hdbextractor-qt.
 *
 * The qdddplot library requires the qgraphicsplot library (Qt QGraphicsView plots), that can be obtained with:
 *
 * \code
 * svn checkout  http://svn.code.sf.net/p/tango-cs/code/gui/qgraphicsplot qgraphicsplot
 * \endcode
 *
 * The qdddplot library can be downloaded with the following command
 *
 * \code
 * svn checkout  http://svn.code.sf.net/p/tango-cs/code/gui/qdddplot qdddplot
 * \endcode
 *
 * The most recent source code of the mathgl project is required (i.e. it must be obtained from the svn
 * development tree).
 *
 * \image html qhdbextractor-3d.png
 *
 * \image latex qhdbextractor-3d.png
 *
 * \par Error visualization.
 *
 * The paint functions of the curves of the qgraphicsplot library draw
 * red vertical lines to represent a NULL value into the database at the given date time.
 * A click close to the invalid data point opens a right pane where error details are available.
 * Please read the hdbextractor-qt documentation manual to learn about the Qt module.
 * These screenshots are taken from the Qt graphical user interface located inside the
 * hdbextractor-qt/QHdbExtractor subfolder.
 *
 * \image html qhdbX-errors.png
 *
 * \image latex qhdbX-errors.png
 *
 * \par Important note
 * Even if the HdbExtractor library is tailored to deal with Tango data (as far as data type,
 * writable property and data format are concerned), it is <em>not dependent</em> from Tango
 * specific data types and libraries at any rate.
 *
 * <h3>Usage</h3>
 *  The use of the Hdbextractor is simple.
 *  The class that uses the Hdbextractor must implement the HdbExtractorListener interface in order
 *  to be notified whenever the data has been partially or completely fetched from the database.
 *  For simplicity, a single interface for accessing data is defined.
 *
 *  When implementing the HdbExtractorListener interface you will have to write the following methods:
 *  <ul>
 *  <li>virtual void onSourceProgressUpdate(const char *name, double percent);</li>
    <li>virtual void onSourceExtractionFinished(const char* name, int totalRows, double elapsed);</li>
    <li>virtual void onExtractionFinished(int totalRows, double elapsed);</li>
 *  </ul>
 *
 * \par Callbacks invoked when data is ready.
 *  Calling Hdbextractor::setUpdateProgressPercent with an integer value greater than 0 determines if
 *  (and when) onSourceProgressUpdate is called in your HdbExtractorListener implementation.
 *  By default, this functionality is disabled (and consequently the method updateProgressPercent
 *  returns -1).<br/>
 *  The HdbExtractorListener::onSourceExtractionFinished and HdbExtractorListener::onExtractionFinished
 *  callbacks are always called when the
 *  extraction of each source is complete and when the whole extraction terminates, respectively.
 *  At that time, you can retrieve the actual data as a whole by calling the Hdbextractor::get method.
 *
 * \par Getting and using extracted data.
 *  When calling get, you have to supply a reference to a std::vector<XVariant>.
 *  Data is always appended to your vector, and the vectory you provide will never be cleared before
 *  this operation.
 *  This allows to accumulate data on the destination vector upon partial data fetch progress
 *  (through onSourceProgressUpdate).
 *  If you skip getting data in onSourceExtractionFinished, you can still copy the whole amount of
 *  extracted values within onExtractionFinished. In this case, you will have to manually separate data
 *  according to the source name stored in the XVariant object. This consideration does not apply if you
 *  are retrieving data one source at the time. In the latter case, it is equivalent to get data in
 *  either of the two callbacks.
 *
 *  \note The HdbExtractorListener::onSourceProgressUpdate is called according to the percentage configured with
 *  Hdbextractor::setUpdateProgressPercent but also when the last bulk of data is available, even if its amount in
 *  terms of percentage is less than the configured value.
 *
 * \par Notes on thread safety and memory management.
 *  Obtaining data with the Hdbextractor::get method is <em>thread safe</em>.
 *  If you opt for partial data fetching, you will invoke Hdbextractor::get inside <em>your implementation of </em>
 *  onSourceProgressUpdate. Every time Hdbextractor::get is called,
 *  the up to that point available data is copied over to the std::vector you supply as parameter of get.
 *  After that data is copied into your vector of XVariant, it is removed from the
 *  Hdbextractor and the memory is freed. <br/>
 *  Subsequent calls to Hdbextractor::get will never hand over data which has already been
 *  copied with prior Hdbextractor::get invocations.
 *  On the other hand, <em>you will not have to worry about data deletion at any point in the
 *  extraction</em> and this <em>remains true for every data structure used in this library</em>,
 *  unless specified. We deem that this greatly simplifies the usage of the library.
 *
 * \par Notes on thread safety and memory management in practise.
 *  Let's briefly discuss what this involves analyzing a use case. Suppose that the the library supplying the plot widgets
 *  provides a method called appendData(string curveName, const vector<double>& timestamps, const vector<double>& data).
 *  This method appends x and y data to the curve with a given curveName. Data is read from the database in the background
 *  by a separate thread. The Hdbextractor has been configured to notify the availability of new data
 *  as soon as a 10 percent of new rows has been fetched from the server.
 *
 *  \code
// New data is available
void MyHdbExtractorImplementation::onSourceProgressUpdate(const char *name , double percent)
{
    // valuelist will host the available data
    std::vector<XVariant> valuelist;

    // get the data in a thread-safe fashion
    // the data fetched from the database is moved into valuelist.
    // This data will not be available the next time get is invoked.
    // valuelist lifetime is limited to the scope of this method.
    // Memory on the Hdbextractor side is automatically managed.
    // mExtractor is a reference to a previously created and configured Hdbextractor
    mExtractor->get(valuelist);

    // vectors to store data to be passed to the plot for drawing
    std::vector<double> timestamps, data;

    // fill them in
    for(size_t i = 0; i < valuelist.size(); i++)
    {
        XVariant v = valuelist[i];
        // use XVariant::getTime_tTimestamp()
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
    MyHdbExtractorImpl(const char *dbuser, const char *dbpass,
                       const char *dbhost, const char *dbnam);

    // fetch data on the database
    void getData(const char* source, const char* start_date, const char *stop_date);

    // HdbExtractorListener pure abstract methods must be implemented (1/3)
    virtual void onSourceProgressUpdate(const char *name, double percent);

    // HdbExtractorListener pure abstract methods must be implemented (2/3)
    virtual void onSourceExtractionFinished(const char* name, int totalRows, double elapsed);

    // HdbExtractorListener pure abstract methods must be implemented (3/3)
    virtual void onExtractionFinished(int totalRows, double elapsed);

    const std::vector<XVariant> &getValuelistRef() const;

private:
    Hdbextractor *mExtractor;
    // will store locally the data fetched
    std::vector<XVariant> d_valuelist;
};

 \endcode
 *
 * Here's the cpp part of MyHdbExtractorImpl
 *
 * \code
 *
MyHdbExtractorImpl::MyHdbExtractorImpl(const char *dbuser, const char *dbpass,
                                       const char *dbhost, const char *dbnam)
{
    printf("trying to connect to host: \"%s\" db name: \"%s\" user: \"%s\"\n", dbhost, dbnam, dbuser);
    // detect the desirde database and schema: HDBMYSQL or HDBPPMYSQL
    mExtractor = new Hdbextractor(this);
    Hdbextractor::DbType type = Hdbextractor::HDBMYSQL;
    if(strcmp(dbnam, "hdb") == 0)
        type = Hdbextractor::HDBMYSQL;
    else if(strcmp(dbnam, "hdbpp") == 0)
        type = Hdbextractor::HDBPPMYSQL;

    bool res = mExtractor->connect(type, dbhost, dbnam, dbuser, dbpass);
    // Get updates when a 10% of the new data is available
    if(res)
        mExtractor->setUpdateProgressPercent(10);
    else
        printf("\nerror connecting to host: %s\n", dbhost);
}

void MyHdbExtractorImpl::getData(std::vector<std::string> sources, const char* start_date, const char *stop_date)
{
    bool res = mExtractor->getData(sources, start_date, stop_date);
    if(!res)
       printf("\e[1;31merror fetching data: %s\e[0m\n", mExtractor->getErrorMessage());
}

void MyHdbExtractorImpl::onSourceExtractionFinished(const char *name, int totalRows, double elapsed)
{
    printf("\"%s\" data extraction completed in %.2fs [%d rows]\n", name, elapsed, totalRows);
}

void MyHdbExtractorImpl::onExtractionFinished(int totalRows, double elapsed)
{
    printf("extraction completed: got %d rows in %fs\n", totalRows, elapsed);
    mExtractor->get(d_valuelist);
}

void MyHdbExtractorImpl::onSourceProgressUpdate(const char *name , double percent)
{
    printf("\"%s\" data extraction: %.2f%%\n", name, percent);
    mExtractor->get(d_valuelist);
}

const std::vector<XVariant> &MyHdbExtractorImpl::getValuelistRef() const
{
    return d_valuelist;
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
        printf("Usage\e[0m \"%s domain/family/member/attribute \"2014-07-20 10:00:00\" \"2014-07-20 12:00:00\"\n",
               argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        const char* start_date = argv[argc - 2];
        const char* stop_date = argv[argc - 1];

        // Gather sources from the command line
        std::vector<std::string> sources;
        for(int i = 2; i < argc - 2; i++)
            sources.push_back(std::string(argv[i]));

        // simply create an instance of our MyHdbExtractorImpl
        // implementation
        //
        MyHdbExtractorImpl *hdbxi = new MyHdbExtractorImpl("hdbbrowser", "hdbbrowser", "srv-log-srf", "hdb");
        hdbxi->getData(sources, start_date, stop_date);

        const std::vector<XVariant> & valuelist = hdbxi->getValuelistRef();
        // ...and call get data with the source name and the start and stop time
        //
        hdbxi->getData(argv[1], argv[2], argv[3]);

        const std::vector<XVariant> & values = hdbxi->getValuelistRef();
        // This method simply prints the values on the console.
        // The second parameter specifies the maximum number of elements to print when XVariant
        // stores vectors.
        XVariantPrinter().printValueList(values, 2);
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
 * <h3>Configuration files</h3>
 * The ConfigurationParser class can be used to store couples key/value in form of
 * strings. The ConfigurationParser reads a text file where the settings are written and saves
 * them into a std::map<std::string, std::string>.
 * A higher level interface to load and get settings from a configuration file is provided by the
 * HdbXSettings class. This provides conversion methods that easily decode the values into
 * integer, double and boolean types in addition to strings.
 * In general, you should directly use HdbXSettings. A description of the configuration file follows.
 * Its format is the same for the ConfigurationParser and the HdbXSettings.
 * The file is in plain text format; here's an example:
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
 * The ConfigurationParser can be used to store the connection parameters and application specific settings as well.
 *
 * To get the configuration pairs, instantiate a ConfigurationParser and call read
 * with the path of the file and a reference to a std::map<std::string, std::string> <br/>
 * If read returns true, the pairs are stored into the map.<br/>
 * Please refer to the ConfigurationParser documentation for an in depth view of the class.
 *
 * @see ConfigurationParser
 * @see ConfigurationParser::read
 *
 * <h3>HdbXSettings</h3>
 * The HdbXSettings provides a higher level interface to configuration files. Convenience functions are
 * provided in order to convert the values into different types of data.
 * <br>Please refer to the HdbXSettings documentation for further details. HdbXSettings should be the
 * way to go as far as key/value based configuration files are concerned.
 *
 * <h3>Query and data fetch configuration</h3>
 *
 * The HdbXSettings class lets you customize queries according to
 * specific needs and database connection characteristics.
 *
 * For example, if no data is stored within a given period of time, let's say between t0 and t1, Hdbextractor
 * can be told to retrieve the most recent recorded data at a time t < t0 instead of simply returning an empty result.
 * In addition, if the first available data inside (t_start, t_end) has been recorded at a time t1 such as
 * (t1 - t_start) is greater than a given fraction of (t_end - t_start), an additional value fetched at
 * t_past = MAX(t : t < t_start) can be inserted in the data set at t_start.
 *
 * Take a look at the figure below. As the dotted curve reveals, there is no data in the database from
 * December, 31st to January, 8th.
 *
 * \image html qhdbextractor-fetch-past-1.png
 *
 * \image latex qhdbextractor-fetch-past-1.png
 *
 * Now let's restrict the time interval on the left only, to include a certain
 * amount of time without recorded data: from January, 2nd to January, 24.
 * As expected, data starts from January, 8th and is displayed starting from that point.
 *
 * \image html qhdbextractor-fetch-past-2.png
 *
 * \image latex qhdbextractor-fetch-past-2.png
 *
 * The described behavior is adopted if the configuration key <em>FillFromThePastMode</em> is set to
 * <em>None</em> in the configuration file:
 *
 * \code

# The FillFromThePastMode configures how the lack of data in the desired time window is managed.
# # Possible values (if not specified, the default is None):
# #
# # None:        nothing is done if the window does not contain data or if valid data starts late.
# # KeepWindow:  look in the past for the most recent valid data and make it the first value of the window.
# #              The timestamp is changed to start the date/time  of the time window.
# # WidenWindow: look in the past for the most recent valid data and put it in as the first result with its
# #              preserving its original timestamp.
# #
FillFromThePastMode = None

 \endcode
 *
 * The snippet of the configuration file quoted above describes the usage of the FillFromThePastMode property.
 *
 * Now change the <em>FillFromThePastMode</em> property value to <em>KeepWindow</em>. Here's the result:
 *
 * \image html qhdbextractor-fetch-past-3.png
 *
 * \image latex qhdbextractor-fetch-past-3.png
 *
 * As one can see, data starts from January, 2nd, which is the start date set on the interval.
 * \nThe time window specified by the user is thus preserved.
 *
 * Changing <em>FillFromThePastMode</em> to <em>WidenWindow</em> produces the result below. In this case,
 * the timestamp of the last recorded data before the desired start date/time is put as the first point in
 * the x axis. \nThe time window specified by the user is widened to match the timestamp of the recorded data.
 * See the picture below.
 *
 * \image html qhdbextractor-fetch-past-4.png
 *
 * \image latex qhdbextractor-fetch-past-4.png
 *
 * Actually, the feature of looking back in the past for a value preceding the start date time requested by
 * the user is controlled by another option named <em>FillFromThePastThresholdPercent</em>.
 * This option is taken into account only if <em>FillFromThePastMode</em> is not <em>None</em>.
 * <em>FillFromThePastThresholdPercent</em>  is a floating point number representing the percentage of the
 * whole time interval (t0, tN) specified by the user. If the first available data in the interval has a
 * time stamp <em>t1 > (tN - t0) * FillFromThePastThresholdPercent / 100.0</em>, then the Hdbextractor
 * seeks data in the past (t < t0) and adjusts the resulting data set date and time according to
 * the  <em>FillFromThePastMode</em> property. The default value of <em>FillFromThePastThresholdPercent</em>
 * is set to 5%.
 *
 *
 * <h3>Sieving results of multiple source queries and data time-alignment</h3>
 *
 * The figure below illustrates the trend of the current of two power supplies over two hours.
 * As expected, each trend evolves independently, in time and current.
 *
 * \image html qhdbextractor-no-siever.png
 *
 * \image latex qhdbextractor-no-siever.png
 *
 * There are times when it may be useful to align two or more trends in time. In these cases, the DataSiever
 * class can help. <br> The DataSiever class
 * <ol>
 * <li>groups data (std::vector<XVariant> ) by source name;</li>
 * <li>fills the data of each source so that each set contains the same values on the x
 *     axis (time). If the source <em>Si</em> has no recorded data at the time <em>tk</em>,
 *     the value saved at the time <em>t < tk</em> is copied at time <em>tk</em>.</li>
 * </ol>
 *
 * As a consequence of (1), DataSiever allows separating data coming from different sources. This
 * feature is useful when you process all the data coming from multiple sources at once (for example,
 * inside the callback onExtractionFinished, skipping per source progress updates).
 *
 * The DataSiever utility class separates the data of each different source (attribute)
 * working on the vector of XVariant returned by Hdbextractor::getData.
 * Once organized by source name, each vector of data can be aligned in time, with the DataSiever::fill
 * method. <br/>
 *
 * The main.cpp file quoted above can be modified as shown below in order to use DataSiever
 * and HdbXSettings. This version requires that the program is launched with an additional file
 * name which stores the settings. Build the hdbxtest source code to run the example.
 *
 * \code
#include <stdio.h>
#include <stdlib.h>
#include "myhdbextractorimpl.h"
#include <hdbextractor.h>

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 5)
    {
        printf("\e[1;31mUsage\e[0m \"%s configfile.dat domain/family/member/attribute \"2014-07-20 10:00:00\" \"2014-07-20 12:00:00\"\n",
               argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        const char* start_date = argv[argc - 2];
        const char* stop_date = argv[argc - 1];

        std::vector<std::string> sources;
        for(int i = 2; i < argc - 2; i++)
            sources.push_back(std::string(argv[i]));

        HdbXSettings *qc = new HdbXSettings();
        qc->loadFromFile(argv[1]);

        MyHdbExtractorImpl *hdbxi = new MyHdbExtractorImpl(qc->get("dbuser").c_str(),
                qc->get("dbpass").c_str(), qc->get("dbhost").c_str(), qc->get("dbname").c_str());

        hdbxi->getHdbExtractor()->setHdbXSettings(qc);    // settings
        hdbxi->getData(sources, start_date, stop_date);   // get data

        const std::vector<XVariant> & valuelist = hdbxi->getValuelistRef();


        DataSiever siever;
        siever.divide(valuelist);
        siever.fill();

        std::vector<std::string> srcs = siever.getSources();
        for(size_t i = 0; i < srcs.size(); i++)
        {
           std::vector<XVariant > values = siever.getData(srcs.at(i));
           XVariantPrinter().printValueList(values, 2);
        }
        delete qc;
    }
    return 0;
}
 * \endcode
 *
 * The screenshot below shows the result of the time fill process. It was taken from the qhdbextractor
 * graphical application, which has an check box to enable the fill time option.
 *
 * \image html  qhdbextractor-fill-time.png
 * \image latex qhdbextractor-fill-time.png
 *
 * See the DataSiever documentation for further details.
 *
 */
class Hdbextractor :  public ResultListener
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

    virtual void setUpdateProgressPercent(int percent);

    /** \brief returns the number of rows after which onProgressUpdate is called on the registered HdbExtractorListener
     *
     * @return the number of rows after which onProgressUpdate is called on the registered HdbExtractorListener
     *
     * @see setUpdateProgressStep
     * @see HdbExtractorListener
     * @see getData
     */
    int updateProgressPercent();

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

    void setHdbXSettings(HdbXSettings *qc);

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

    virtual void onProgressUpdate(const char* name, double percent);

    virtual void onFinished(int totalRows, double elapsed);

    virtual void onSourceExtracted(const char *source, int totalRows, double elapsed);

    const char* getErrorMessage() const;

    bool hasError() const;

    HdbXSettings *getHdbXSettings() const;

    void cancelExtraction();

    bool extractionIsCancelled() const;

private:
    HdbExtractorPrivate *d_ptr;
};

#endif // HDBEXTRACTOR_H
