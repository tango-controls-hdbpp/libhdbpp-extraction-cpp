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

    virtual ~HdbExtractorListener() {}

    /** \brief this method is invoked according to the percentage value configured in setUpgradeProgressPercent
     *         whenever the specified percentage of rows are read from the database.
     *
     * \note By default, if numRows is not set, onProgressUpdate is not invoked and the results
     *       are available when onFinished is invoked.
     *
     * @param name the name of the tango device/attribute being extracted
     * @param percent the percentage of the completed steps
     *
     * @see onSourceExtracted
     * @see onSourceExtractionFinished
     * @see onSourceProgressUpdate
     */
    virtual void onSourceProgressUpdate(const char *name, double percent) = 0;

    /** \brief this method is invoked when data extraction is fully accomplished on the source with the given name
     *
     *
     * @param totalRows the number of rows extracted from the database.
     * @param elapsed the time elapsed (seconds.microseconds) since extraction started.
     *
     * @see onSourceExtractionFinished
     */
    virtual void onExtractionFinished(int totalRows, double elapsed) = 0;

    /** \brief this method is invoked when data extraction is fully accomplished on the source with the given name
     *
     *
     * @param name the name of the source whose extraction is complete.
     * @param totalRows the number of rows extracted for the give source.
     * @param elapsed the time elapsed (seconds.microseconds) needed for source extraction.
     *
     * @see onSourceProgressUpdate
     * @see onSourceExtractionFinished
     */
    virtual void onSourceExtractionFinished(const char *name, int totalRows, double elapsed) = 0;
};

#endif // HDBEXTRACTORLISTENER_H
