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
    struct timeval timeva, tv1, tv0;

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
            timestamp_set.insert(timestamp);
        }
    }

    tstamps_size = timestamp_set.size();
    size_t tsidx = 0, dataidx;

    XVariantPrinter printer;

    /* for each data row */
//    for(std::map<std::string, std::vector<XVariant> >::iterator it = d_ptr->dataMap.begin();
//        it != d_ptr->dataMap.end(); ++it)
    for(std::map<std::string, std::vector<int> >::iterator testit = d_ptr->testMap.begin();
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

        std::vector<XVariant> tmpdata;
        /// data.reserve(tstamps_size);
        /* create an iterator over data */
///        std::vector<XVariant>::iterator datait = data.begin();
///        std::vector<XVariant>::iterator dataend = data.end();
///        datait = datait + 1; /* will start from the second element */

        std::vector<int>::iterator testdit = testd.begin();
        testdit = testdit + 1;
        while(datait != dataend)
        {
            /* end of interval */
            tv1 = datait->getTimevalTimestamp();
            data_timestamp_1 = tv1.tv_sec + tv1.tv_usec * 1e-6;
            /* start of interval */
            tv0 = (datait - 1)->getTimevalTimestamp();
            data_timestamp_0 = tv0.tv_sec + tv0.tv_usec * 1e-6;

            /* iterate over the timestamps stored in the timestamp set. As we walk the set, avoid
             * searching the same interval multiple times. For this, keep ts_set_iterator as
             * start and update it in the last else if branch.
             */
            std::set<double>::iterator tsiter = ts_set_iterator;
            while(tsiter != timestamp_set.end())
            {
                time_t tt = (time_t) (*tsiter);
                printf("set %ld/%ld: %s %s\n", tsidx, tstamps_size, ctime(&tt), datait->getSource());
                tsidx++;
                if((*tsiter) >  data_timestamp_0 && (*tsiter) < data_timestamp_1)
                {
                    printf("+++ building temp xvariant data siz %ld:\n", (*datait).getSize());
                    XVariant xvariant(*datait);
                    xvariant.setTimestamp(*tsiter);
                    /* insert before the position of the iterator datait */
                    printf("\e[0;32m + %p data idx %ld data siz %ld filled %s[%s] with %s\e[0m",
                           &data, dataidx, data.size(),
                           (*datait).getSource(),
                           (*datait).getTimestamp(),
                           ctime(&tt));
                    printer.print((*datait), 2);
                    printf("++++ inserting into data\n");
                    data.insert(datait, xvariant);
                    tmpdata.push_back(xvariant);
                    printf("src %s size %ld fmt %d ts %s\n", data[dataidx - 1].getSource(),
                            data[dataidx - 1].getSize(),
                           data[dataidx -1 ].getFormat(), data[dataidx -1 ].getTimestamp());
                    printf("src %s size %ld fmt %d ts %s\n", data[dataidx + 1].getSource(),
                            data[dataidx - 1].getSize(),
                           data[dataidx -1 ].getFormat(), data[dataidx +1 ].getTimestamp());
                  //  datait = data.begin() + dataidx;
                    printf("\e[1;34m %p rite after insert data size %ld:\e[0m\n", &data, data.size());
                    for(int k =0 ; k < data.size(); k++)
                        printer.print(data[k], 2);
                    printf("\e[1;34m %p rite after insert, but from xvariant:\e[0m\n", &data);
                    printer.print(xvariant, 2);

                    ///return;
//                    printf("\e[1;32m + %p data idx %ld data siz %ld filled %s [%s] with %s\e[0m",
//                           &data, dataidx, data.size(), (*datait).getSource(), (*datait).getTimestamp(), ctime(&tt));

                    printf("\e[1;32m + %p data idx %ld data siz %ld filled  with %s\e[0m",
                            &data, dataidx, data.size(), /*(*datait). getSource(),*/
                            ctime(&tt));
                    printer.print((*datait), 2);
                    dataidx++;
                    datait = data.begin() + dataidx;
                }
                else if((*tsiter) == data_timestamp_1) /* simply skip */
                {
                    printf("\e[1;35m - skipping element cuz equal to timestamp_1: %s\e[0m", ctime(&tt));
                 //   tsiter++;
                    tsiter++;
                    ts_set_iterator = tsiter; /* point to next */
                    tmpdata.push_back((*datait));
                    break;
                }
                else if((*tsiter) > data_timestamp_1)
                {
                    ts_set_iterator = tsiter; /* save to optimize next for */
                    printf("\e[1;32m > going to next point after %s \e[0m", ctime(&tt));
                    break;
                }
                tsiter++;
            }
            dataidx++;
            printf("\e[1;34m data index %ld data size %ld data %p\e[0m\n", dataidx, data.size(), &data);
            datait = data.begin() + dataidx;
            dataend = data.end();
            //// datait++;
        }
        d_ptr->dataMap[it->first] = tmpdata;
        printer.printValueList(d_ptr->dataMap[it->first], 2);
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


