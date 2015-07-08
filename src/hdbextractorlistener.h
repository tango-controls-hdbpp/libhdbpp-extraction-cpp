#ifndef HDBEXTRACTORLISTENER_H
#define HDBEXTRACTORLISTENER_H

class XVariantList;

/** \brief An interface defining methods to obtain results from the query as long as they are available,
 *         also partially. Its methods are thread safe.
 *
 */
class HdbExtractorListener
{
public:
    HdbExtractorListener() {}

    /** \brief this method is invoked according to the numRows value configured in setUpgradeProgressStep
     *         whenever numRows rows are read from the database.
     *
     * \note By default, if numRows is not set, onProgressUpdate is not invoked and the results
     *       are available when onFinished is invoked.
     *
     * @param name the name of the tango device/attribute being extracted
     * @param step the number of processed rows
     * @param totalSteps the number of data rows for the source with name name
     *
     * @see onSourceExtracted
     */
    virtual void onSourceProgressUpdate(const char *name, int step, int totalSteps) = 0;

    /** \brief this method is invoked when data extraction is fully accomplished on the source with the given name
     *
     *
     * @param name the name of the tango device/attribute just extracted.
     * @param sourceStep the number of sources extracted up to now
     * @param sourcesTotal the number of sources involving the data extraction.
     * @param elapsed the time elapsed (seconds.microseconds) since extraction started.
     *
     * @see onSourceProgressUpdate
     */
    virtual void onSourceExtracted(const char * name, int sourceStep, int sourcesTotal, double elapsed) = 0;
};

#endif // HDBEXTRACTORLISTENER_H
