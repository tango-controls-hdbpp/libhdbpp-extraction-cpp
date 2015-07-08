#include "myhdbextractorimpl.h"
#include <stdio.h>
#include <string.h>
#include "../src/hdbextractor.h"
#include "../src/xvariant.h"

MyHdbExtractorImpl::MyHdbExtractorImpl(const char *dbuser, const char *dbpass,
                                       const char *dbhost, const char *dbnam)
{
    printf("\033[0;37mtrying to connect to host: \"%s\" db name: \"%s\" user: \"%s\"\033[0m\t", dbhost, dbnam, dbuser);

    mExtractor = new Hdbextractor(this);
    Hdbextractor::DbType type;
    if(strcmp(dbnam, "hdb") == 0)
        type = Hdbextractor::HDBMYSQL;
    else if(strcmp(dbnam, "hdbpp") == 0)
        type = Hdbextractor::HDBPPMYSQL;

    bool res = mExtractor->connect(type, dbhost, dbnam, dbuser, dbpass);
    if(res)
    {
        printf("\e[1;32mOK\e[0m\n");
        mExtractor->setUpdateProgressStep(20);
    }
    else {
        printf("\e[1;31merror connecting to host: %s\e[0m\n", dbhost);
    }

}

void MyHdbExtractorImpl::getData(std::vector<std::string> sources, const char* start_date, const char *stop_date)
{

    bool res = mExtractor->getData(sources, start_date, stop_date);
    if(!res)
    {
        for(int i = 0; i < sources.size(); i++)
            printf("\e[1;31merror fetching data: %s: %s\e[0m\n", sources[i].c_str(), mExtractor->getErrorMessage());
    }
}

/** \brief this method is invoked according to the numRows value configured in setUpgradeProgressStep
 *         whenever numRows rows are read from the database.
 *
 * \note By default, if numRows is not set, onProgressUpdate is not invoked and the results
 *       are available when onFinished is invoked.
 *
 * @see onFinished
 */
void MyHdbExtractorImpl::onSourceProgressUpdate(const char *name, int step, int total)
{
    printf("\"%s\" data extraction: %.2f%% [%d/%d]\n", name, (float)step / total * 100.0, step, total);
    extractData();
}

/** \brief this method is invoked when data extraction is fully accomplished.
 *
 */
void MyHdbExtractorImpl::onSourceExtracted(const char * name, int sourceStep, int sourcesTotal, double elapsed)
{
    printf("extraction completed: got %d rows from \"%s\" in %fs\n", sourceStep, name, elapsed);
    extractData();
}

void MyHdbExtractorImpl::extractData()
{
    mExtractor->get(d_valuelist);

//    for(size_t i = 0; i < valuelist.size(); i++)
//    {
//        XVariant::DataFormat format = valuelist[i].getFormat();
//        if(format == XVariant::Scalar)
//        {
//            printf("%s -> \e[1;32m%.2f\e[0m], ", valuelist[i].getTimestamp(), valuelist[i].toDouble());
//            if(i > 0 && i % 20 == 0)
//                printf("\n");
//        }
//        else if(format == XVariant::Vector)
//        {
//            std::vector<double> values = valuelist[i].toDoubleVector();
//            if(valuelist.size() > 0)
//                printf("\e[1;33m[ %s\e[0m", valuelist[i].getTimestamp());
//            for(size_t j = 0; j < values.size(); j++)
//                printf("\e[0;35m%ld:\e[1;32m %g\e[0m ,", j, values[j]);
//            printf(" \e[1;33m]\e[0m\n");

//        }
//    }
//    printf("\n\n");
}

const std::vector<XVariant> &MyHdbExtractorImpl::getValuelistRef() const
{
    return d_valuelist;
}

