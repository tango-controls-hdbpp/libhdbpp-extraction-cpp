#include "datasiever.h"
#include "datasieverprivate.h"
#include "xvariantprinter.h"

#include <string.h>
#include <set>
#include <time.h>
#include <hdbxmacros.h>

#define MAXSRCLEN 256

DataSiever::DataSiever()
{
    d_ptr = new DataSieverPrivate();
    d_ptr->minTimestamp = 0.0;
}

DataSiever::~DataSiever()
{
    //d_ptr->dataMap.clear();
    //delete d_ptr;
}

void DataSiever::clear()
{
    d_ptr->dataMap.clear();
}

/** \brief Separates data coming from different sources, saving couples source name / data in an internal
 *         map.
 *
 * @param rawdata a vector of XVariant representing historical data. rawdata can contain the full data,
 *        after it has previously been completely fetched, or just a partial result. In the latter case,
 *        divide can be called multiple times and the result is equivalent.
 *
 * The only constraint is that rawdata contains data ordered by time for each one of the different sources.
 * The internal map used to save source names and their data values is not cleared by divide. For this reason,
 * it is possible to make multiple calls to this method whenever a new chunk of data is available from the
 * database.
 *
 */
void DataSiever::divide(const std::vector<XVariant> &rawdata)
{
    char source[MAXSRCLEN] = "";
    size_t i;
    size_t dataSize = rawdata.size();
    /* see which row's data is most dated */
    double timestamp;
    struct timeval tval;

    for(i = 0; i < dataSize; i++)
    {
        XVariant xv = rawdata[i];
        const char *src = xv.getSource();
        if(strcmp(src, source) != 0 && d_ptr->dataMap.count(std::string(src)) == 0)
        {
            d_ptr->dataMap[std::string(src)] = std::vector<XVariant>();
            strncpy(source, src, MAXSRCLEN);
            /* take the opportunity to save min timestamp */
            tval = xv.getTimevalTimestamp();
            timestamp = tval.tv_sec + tval.tv_usec * 1e-6;
            if(d_ptr->minTimestamp == 0.0 || timestamp < d_ptr->minTimestamp)
            {
                d_ptr->minTimestamp = timestamp;
            }
        }
        printf("pushing back on \"%s\"\n", source);
        d_ptr->dataMap[std::string(source)].push_back(xv);
        d_ptr->testMap[std::string(source)].push_back(xv.getTime_tTimestamp());
    }

}

void DataSiever::fill()
{
    size_t i, tstamps_size, ts_i;
    double timestamp, data_timestamp_0, data_timestamp_1;
    /// struct timeval timeva, tv1, tv0;
    struct timeval timeva;
    time_t tv1, tv0;
    /* create a std set (always sorted following a specific strict weak
     * ordering criterion indicated by its internal comparison object)
     * with all the required timestamps that will fill the n vectors of
     * data.
     */
    std::set<double> timestamp_set;
    for(std::map<std::string, std::vector<XVariant> >::iterator it = d_ptr->dataMap.begin();
        it != d_ptr->dataMap.end(); ++it)
    {
        std::vector<XVariant> &data = it->second;
        for(i = 0; i < data.size(); i++)
        {
            timeva = data[i].getTimevalTimestamp();
            timestamp = timeva.tv_sec + timeva.tv_usec * 1e-6;
            /* insert timestamp in the set. If timestamp is duplicate, it's not inserted */
            time_t ts = data[i].getTime_tTimestamp();
            printf("...inserting timestamp %d->%s", data[i].getTime_tTimestamp(), ctime(&ts));
            timestamp_set.insert(ts);
        }
    }

    tstamps_size = timestamp_set.size();
    size_t tsidx = 0, dataidx;

    XVariantPrinter printer;

    /* for each data row */
//    for(std::map<std::string, std::vector<XVariant> >::iterator it = d_ptr->dataMap.begin();
//        it != d_ptr->dataMap.end(); ++it)
    for(std::map<std::string, std::vector<time_t> >::iterator testit = d_ptr->testMap.begin();
        testit != d_ptr->testMap.end(); ++testit)
    {

        bool done = false;
        tsidx = 0;
        dataidx = 1;
        std::set<double>::iterator ts_set_iterator = timestamp_set.begin();
        ts_i = 0;
        /* take the vector of data from the map */
        /// std::vector<XVariant> &data = it->second;

        ///
        std::vector<time_t> &testd = testit->second;
        ///

        /* create an iterator over data */

        std::vector<time_t>::iterator testdit = testd.begin() + 1;
        while(testdit != testd.end())
        {
            /* end of interval */
            tv1 = (*testdit);
            /* start of interval */
            tv0 = (*(testdit - 1));

            /* iterate over the timestamps stored in the timestamp set. As we walk the set, avoid
             * searching the same interval multiple times. For this, keep ts_set_iterator as
             * start and update it in the last else if branch.
             */
            std::set<double>::iterator tsiter = ts_set_iterator;
            while(tsiter != timestamp_set.end())
            {
                time_t tt = (time_t) (*tsiter);
                printf("... %s evaluating if %f (%ld) in %ld - %ld\n", testit->first.c_str(), (*tsiter), tt, tv0, tv1);
                printf("... that is %s", ctime(&tt));
                printf("...   between %s", ctime(&tv0));
                printf("...   and %s", ctime(&tv1));
                if((*tsiter) >  tv0 && (*tsiter) < tv1)
                {
                    printf("+++ inserting data siz %ld:\n", testd.size());
                    testdit = testd.insert(testdit, (*tsiter));
                    printf("+++ inserted data siz %ld:\n", testd.size());
                }
                else if(tt == tv1) /* simply skip */
                {
                    printf("\e[1;35m - skipping element cuz equal to timestamp_1: %s\e[0m", ctime(&tt));
                    tsiter++;
                    ts_set_iterator = tsiter; /* point to next */
                    break;
                }
                else if(tt > tv1)
                {
                  //  tsiter++;
                    ts_set_iterator = tsiter; /* save to optimize next for */
                    printf("\e[1;32m > going to next point after %s \e[0m", ctime(&tt));
                    break;
                }
                tsiter++;
                ts_set_iterator = tsiter; /* save to optimize next for */
            }
            testdit++;
            printf("\e[1;34m data index %ld data size %ld data %p\e[0m\n", dataidx, testd.size(), &testd);
        }
    }

    for(std::map<std::string, std::vector<time_t> >::iterator testit = d_ptr->testMap.begin();
        testit != d_ptr->testMap.end(); ++testit)
    {
        printf("\e[0;36m\"%s\"\e[0m\n", testit->first.c_str());
        std::vector<time_t> &tts = testit->second;
        for(int i = 0; i < tts.size(); i++)
            printf("\e[0;36m%s, ", ctime(&tts[i]));
        printf("\e[0m\n\n");
    }


}

size_t DataSiever::getSize() const
{
    return d_ptr->dataMap.size();
}

bool DataSiever::contains(std::string source) const
{
    return d_ptr->dataMap.count(source) > 0;
}

std::vector<std::string> DataSiever::getSources() const
{
    std::vector<std::string> srcs;
    for(std::map<std::string, std::vector<XVariant> >::iterator it = d_ptr->dataMap.begin();
        it != d_ptr->dataMap.end(); ++it)
        srcs.push_back(it->first);
    return srcs;
}

std::vector<XVariant> DataSiever::getData(std::string source) const
{
    std::vector<XVariant>  ret;
    if(d_ptr->dataMap.count(source) > 0)
        ret = d_ptr->dataMap[source];
    return ret;
}

const std::map<std::string, std::vector<XVariant> > & DataSiever::getDataRef() const
{
    const std::map<std::string, std::vector<XVariant> > &rData = d_ptr->dataMap;
    return rData;
}

std::map<std::string, std::vector<XVariant> > DataSiever::getData() const
{
    return d_ptr->dataMap;
}


