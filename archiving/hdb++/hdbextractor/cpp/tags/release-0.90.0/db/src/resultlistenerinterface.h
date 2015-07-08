#ifndef RESULTLISTENER_H
#define RESULTLISTENER_H

class XVariantList;


/** \brief Defines an interface to receive notifications from the database proxy manager (DbSchema
 *         implementation) when data is available. Used internally.
 *
 * This interface and its implementations are used internally.
 *
 * Users are encouraged to read the HdbExtractorListener documentation.
 *
 * @see HdbExtractorListener
 *
 */
class ResultListener
{
public:
    ResultListener() {}

    /** \brief set the number of rows after which a progress update must be triggered on the listener.
     *
     * @param numRows every numRows onProgressUpdate is invoked
     *
     * @see onProgressUpdate
     */
    virtual void setUpdateProgressStep(int numRows) = 0;

    /** \brief this method is invoked according to the numRows value configured in setUpgradeProgressStep
     *         whenever numRows rows are read from the database.
     *
     * \note By default, if numRows is not set, onProgressUpdate is not invoked and the results
     *       are available when onFinished is invoked.
     *
     * @param source the name of the tango attribute whose extraction progress is being reported
     * @param step the row extracted over the total rows for the given source
     * @param total the total number of rows for the source in the database
     *
     * @see setUpdateProgressStep
     * @see onFinished
     */
    virtual void onProgressUpdate(const char *source, int step, int total) = 0;

    /** \brief this method is invoked when data extraction is fully accomplished.
     *
     * @param source the name of the tango attribute
     * @param sourceStep the source completely extracted over the total sources
     * @param totalSources the number of requested sources to be extracted
     * @param elapsed the time that has elapsed since the operation begun.
     */
    virtual void onFinished(const char* source, int sourceStep, int totalSources, double elapsed) = 0;
};

#endif // RESULTLISTENER_H
