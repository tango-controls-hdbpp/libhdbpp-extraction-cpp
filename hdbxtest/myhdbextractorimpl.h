#ifndef MYHDBEXTRACTORIMPL_H
#define MYHDBEXTRACTORIMPL_H

#include "../src/hdbextractorlistener.h"

class Hdbextractor;

#include <vector>
#include <xvariant.h>

/** \brief an <em>example</em> of an implementation of the HdbExtractorListener
 *
 */
class MyHdbExtractorImpl : public HdbExtractorListener
{
public:
    MyHdbExtractorImpl(const char *dbuser, const char *dbpass,
                       const char *dbhost, const char *dbnam);

    virtual ~MyHdbExtractorImpl();

    void getData(std::vector<std::string> sources, const char* start_date, const char *stop_date);

    virtual void onSourceProgressUpdate(const char *name, double percent);

    virtual void onExtractionFinished(int totalRows, double elapsed);

    virtual void onSourceExtractionFinished(const char* name, int totalRows, double elapsed);

    Hdbextractor* getHdbExtractor() const { return mExtractor; }

    void extractData();

    const std::vector<XVariant> &getValuelistRef() const;

private:
    Hdbextractor *mExtractor;

    std::vector<XVariant> d_valuelist;
};

#endif // MYHDBEXTRACTORIMPL_H
