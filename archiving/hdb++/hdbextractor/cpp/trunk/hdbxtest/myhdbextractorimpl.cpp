#include "myhdbextractorimpl.h"
#include <stdio.h>
#include <string.h>
#include "../src/hdbextractor.h"
#include "../src/db/xvariant.h"

MyHdbExtractorImpl::MyHdbExtractorImpl(const char *dbuser, const char *dbpass,
                                       const char *dbhost, const char *dbnam)
{
    printf("\033[0;37mtrying to connect to host: \"%s\" db name: \"%s\" user: \"%s\"\033[0m\t", dbhost, dbnam, dbuser);

    mExtractor = new Hdbextractor(this);
    Hdbextractor::DbType type = Hdbextractor::HDBMYSQL;
    if(strcmp(dbnam, "hdb") == 0)
        type = Hdbextractor::HDBMYSQL;
    else if(strcmp(dbnam, "hdbpp") == 0)
        type = Hdbextractor::HDBPPMYSQL;

    bool res = mExtractor->connect(type, dbhost, dbnam, dbuser, dbpass);
    if(res)
    {
        printf("\e[1;32mOK\e[0m\n");
        mExtractor->setUpdateProgressPercent(10);
    }
    else {
        printf("\e[1;31merror connecting to host: %s\e[0m\n", dbhost);
    }

}

MyHdbExtractorImpl::~MyHdbExtractorImpl()
{
        printf("DELETING mExtractor\n");
        delete mExtractor;
}

void MyHdbExtractorImpl::getData(std::vector<std::string> sources, const char* start_date, const char *stop_date)
{

    bool res = mExtractor->getData(sources, start_date, stop_date);
    if(!res)
    {
        for(size_t i = 0; i < sources.size(); i++)
            printf("\e[1;31merror fetching data: %s: %s\e[0m\n", sources[i].c_str(), mExtractor->getErrorMessage());
    }
}

void MyHdbExtractorImpl::onSourceExtractionFinished(const char *name, int totalRows, double elapsed)
{
    printf("\"%s\" data extraction completed in %.2fs [%d rows]\n", name, elapsed, totalRows);
}


/** \brief this method is invoked when data extraction is fully accomplished.
 *
 */
void MyHdbExtractorImpl::onExtractionFinished(int totalRows, double elapsed)
{
    printf("extraction completed: got %d rows in %fs\n", totalRows, elapsed);
    extractData();
}

/** \brief this method is invoked according to the percentage value configured in setUpgradeProgressPercent.
 *
 * \note By default, if percentage is less than or equal to 0, onProgressUpdate is not invoked and the results
 *       are available when onExtractionFinished is invoked.
 *
 * @see onExtractionFinished
 */
void MyHdbExtractorImpl::onSourceProgressUpdate(const char *name , double percent)
{
    printf("\"%s\" data extraction: %.2f%%\n", name, percent);
    extractData();
}

void MyHdbExtractorImpl::extractData()
{
    mExtractor->get(d_valuelist);
}

const std::vector<XVariant> &MyHdbExtractorImpl::getValuelistRef() const
{
    return d_valuelist;
}

