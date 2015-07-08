#ifndef QHDBEXTRACTORPROXY_H
#define QHDBEXTRACTORPROXY_H

#include "hdbextractor-qt_global.h"

#include <QObject>
#include <hdbextractor.h>

class QHdbextractorProxyPrivate;
class QDateTime;
class QHdbNewDataEvent;
class QHdbNewErrorDataEvent;
class QHdbDataBoundaries;

/** \mainpage qhdbextractorproxy HdbExtractor Qt module
 *
 * This section describes how hdbextractor can be included and easily used by your
 * Qt application.
 *
 * To start with an example, please take a look at the QHdbExtractor graphical user interface.
 *
 * \par Configuration
 * \image html qhdbextractor-config.png
 *
 * \par Scalar data visualization
 * \image html qhdbextractor.png
 *
 * \par Error visualization
 * Red vertical lines represent a NULL value into the database at the corresponding date time.
 * A click close to the invalid data point opens a right pane where error details are available.
 *
 * \image html qhdbX-errors.png
 */

/** \brief This class extends HdbExtractor and implements HdbExtractorListener in order to
 *         provide a high level object capable of fetching data from a database and handing
 *         it to the user by means of a pattern familiar to Qt programmers.
 *
 * Signals are emitted during extraction progress and when data fetch finishes.
 * There are signals transmitting the progress of the extraction and signals
 * carrying the extracted data, partial or as a whole.
 * It is safe to connect the signals emitted by QHdbextractorProxy to the main thread
 * of your application (the UI thread). QHdbextractorProxy takes care of emitting the
 * signals in the main thread of QApplication. This means that the default Qt DirectConnection
 * can be used in signal/slot connections.
 *
 */
class HDBEXTRACTORQTSHARED_EXPORT QHdbextractorProxy : public QObject
{
    Q_OBJECT

public:
    QHdbextractorProxy(QObject *parent = NULL);

    virtual ~QHdbextractorProxy();

    Hdbextractor *getHdbExtractor() const;

    void setUpdateProgressStep(int numRows);

    int updateProgressStep() const;

    void connect(Hdbextractor::DbType dbType,
                 const QString& host,
                 const QString& db,
                 const QString& user,
                 const QString& passwd,
                 unsigned short port = 3306);


    void printData(const std::vector<XVariant>& data);

public slots:

    void getData(const QStringList &sources,
                 const QDateTime &start_date,
                 const QDateTime &stop_date);

    void getErrors(const QString& source, double startTime, double stopTime);

    void getData(const QString& source, const QDateTime& startDate, const QDateTime& stop_date);

    void disconnect();

    void getSourcesList();

protected:

    void dataErrorNotify(QHdbNewErrorDataEvent *e);

    void dataNotify(QHdbNewDataEvent *e);

signals:
    void onConnected(bool success);

    /** \brief This signal notifies that data fetch has reached step steps over a the total
     *
     * @param steps the progress of data extraction over the total
     * @param the total rows of the extraction.
     *
     * \note The signal is delivered from the main thread and can be safely used in the UI thread.
     */
    void sourceExtractionProgress(const QString& source, int step, int totalSteps);

    /** \brief This signal notifies that the extraction is complete.
     *
     * This signal is emitted also when setUpdateProgressStep has been called with a number of
     * rows greater than zero (that is, when also progress is periodically emitted).
     *
     * @param totalRows the number of rows read from the database
     *
     * \note The signal is delivered from the main thread and can be safely used in the UI thread.
     */
    void sourceExtractionFinished(const QString& source, int sourceStep, int totalSources, double timeElapsed);

    /** \brief This signal is emitted when new <em>read only or write only</em> scalar data is available
     *
     * @param source the tango point extracted from the database (domain/family/member/attribute_name)
     * @param timestamps vector of timestamps to be used in X axis (time)
     * @param data vector of data aligned with the timestamps
     *
     * \note
     * Since the underlying Hdbextractor is thread-safe, there is no need to queue connection between
     * QHdbextractorThread::dataReady and your slot.
     * The signal is delivered from the main thread and can be safely used in the UI thread.
     */
    void dataReady(const QString& source, const QVector<double>& timestamps, const QVector<double> & data);


    /** \brief This signal is emitted when new <em>read write</em> scalar data is available
     *
     * @param source the tango point extracted from the database (domain/family/member/attribute_name)
     * @param timestamps vector of timestamps to be used in X axis (time)
     * @param read_data vector of data aligned with the timestamps (Tango read value)
     * @param write_data vector of data aligned with the timestamps (Tango write value)
     *
     * \note
     * Since the underlying Hdbextractor is thread-safe, there is no need to queue connection between
     * QHdbextractorProxy::dataReady and your slot.
     * The signal is delivered from the main thread and can be safely used in the UI thread.
     *
     * Read data and write data are fetched together for each source, so they are sent
     * conjoined by this signal.
     */
    void dataReady(const QString& source, const QVector<double>& timestamps,
                   const QVector<double> & read_data,
                   const QVector<double> & write_data);


    /** \brief This signal is emitted when new vector of data is available
     *
     * @param source the tango point extracted from the database (domain/family/member/attribute_name)
     * @param timestamp the timestamp of the data
     * @param data vector of data that was saved at time timestamp
     *
     * \note
     * Since the underlying Hdbextractor is thread-safe, there is no need to queue connection between
     * QHdbextractorProxy::dataReady and your slot.
     * The signal is delivered from the main thread and can be safely used in the UI thread.
     */
    void dataReady(const QString& source, double timestamp, const QVector<double> & data);

    void dataReady(const QString& source,
                   const double *data,
                   size_t dataCount,
                   size_t dataSize,
                   const QHdbDataBoundaries &dataBoundaries);

    /** \brief This signal is emitted when the error information is ready for the given source,
     *         in correspondance of the given timestamps.
     *
     * @param source the name of the source whose error information is transmitted by this signal
     * @param timestamps a vector of timestamps, as double representing seconds.microseconds
     * @param codes a vector of error codes (quality factor). Its elements are aligned with the
     *        timestamps and the messages
     * @param messages a list of error descriptions, aligned with the timestamps and the error
     *        codes.
     *\note
     * Since the underlying Hdbextractor is thread-safe, there is no need to queue connection between
     * QHdbextractorProxy::errorsReady and your slot.
     * Data is extracted from the XVariant object.
     * The signal is delivered from the main thread and can be safely used in the UI thread.
     *
     */
    void errorsReady(const QString& source, const QVector<double>& timestamps,
                     const QVector<int>& code,
                     const QStringList& messages);

    /** \brief This signal is emitted whenever an error occurs in data fetch or extraction
     *
     * @param message The error message
     *
     * \note The signal is emitted in the main QApplication thread.
     */
    void errorOccurred(const QString&  message);

    /** \brief This signal is emitted when the list of historical database sources has been retrieved
     *  from the database.
     *
     * @param srclist the list of sources
     */
    void sourcesListReady(const QStringList& srclist);


private slots:

    /** \brief This slot is connected to the QHdbExtractorThread
     */
    void onUpdate(const QString& srcname, int step, int totalSteps);

    /** \brief This slot is connected to the QHdbExtractorThread
     */
    void onFinish(const QString &srcname, int srcStep, int totSrcs, double elapsed);

    /** \brief This slot is connected to the QHdbExtractorThread
     */
    void onError(const QString& message);

    void onSourcesListReady(const QStringList& srcs);

protected:
    bool event(QEvent *e);

private:
    QHdbextractorProxyPrivate *d_ptr;

};

#endif // QHDBEXTRACTOR_H
