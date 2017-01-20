#include "datasiever.h"
#include "datasieverprivate.h"
#include "xvariantprinter.h"

#include <string.h>
#include <set>
#include <sys/time.h>
#include "../hdbxmacros.h"
#include <unistd.h>

#define MAXSRCLEN 256

DataSiever::DataSiever()
{
    d_ptr = new DataSieverPrivate();
}

DataSiever::~DataSiever()
{
    clear();
    delete d_ptr;
}

void DataSiever::clear()
{
    d_ptr->dataMap.clear();
}

double DataSiever::getElapsedTimeMicrosecs() const
{
    return d_ptr->elapsedMicros;
}

/** \brief Separates data coming from different sources, saving couples source name / data in an internal
 *         map.
 *
 * @param rawdata a vector of XVariant representing historical data. rawdata can contain the full data,
 *        after it has previously been completely fetched, or just a partial result. In the latter case,
 *        divide can be called multiple times and the result is equivalent.
 *
 * The only constraint is that rawdata contains data ordered by time for each one of the different sources
 * (i.e. rawdata is the result of Hdbextractor::getData).
 * The internal map used to save source names and their data values is not cleared by divide. For this reason,
 * it is possible to make multiple calls to this method whenever a new chunk of data is available from the
 * database.
 *
 */
void DataSiever::divide(const std::vector<XVariant> &rawdata)
{
    char source[MAXSRCLEN] = "";

    for(std::vector<XVariant>::const_iterator it = rawdata.begin(); it != rawdata.end(); it++)
    {
        const char *src = it->getSource();
        if(d_ptr->dataMap.count(std::string(src)) == 0)
        {
            d_ptr->dataMap[std::string(src)] = std::list<XVariant>();
            strncpy(source, src, MAXSRCLEN);
        }
        d_ptr->dataMap[std::string(src)].push_back(*it); /* src, not source here! */
    }
}

/** \brief This method, that must be invoked after divide, extends the data of the sources so that
 *         each one has the same points on the time axis.
 *
 * Suppose that s and t are the names of the two sources with some data over time.
 * Let source s have data at t0, t4 and t9 and source t at t1,t3,t4 and t8.
 * The fill method will make s and t time scales equal. s and t will have points at times t0,t1,t3,t4,t8 and t9.
 * The values of s in t1,t2 and t3 will be the same as the value at t0 and the value of s in t8 will be the same
 * as the value in t4, and so on.
 * This time filling routine is useful if you want to deal with time-aligned results.
 *
 * \note divide must be called before fill
 *
 * It is possible to attach a DataSieverListener in order to be notified of the progress in data
 * filling. It may be a time consuming process though.
 *
 * @see divide
 */
void DataSiever::fill()
{
    size_t tstamps_size, step = 0, steps_to_estimate, total_steps;
    double timestamp, data_timestamp_0, data_timestamp_1;
    struct timeval timeva, tv1, tv0, started_tv, ended_tv;
    gettimeofday(&started_tv, NULL);

    /* create a std set (always sorted following a specific strict weak
     * ordering criterion indicated by its internal comparison object)
     * with all the required timestamps that will fill the n vectors of
     * data.
     */
    std::set<double> timestamp_set;
    for(std::map<std::string, std::list<XVariant> >::iterator it = d_ptr->dataMap.begin();
        it != d_ptr->dataMap.end(); ++it)
    {
        std::list<XVariant> &data = it->second;
        for(std::list<XVariant>::iterator it = data.begin(); it != data.end(); it++)
        {
            timeva = it->getTimevalTimestamp();
            timestamp = timeva.tv_sec + timeva.tv_usec * 1e-6;
            /* insert timestamp in the set. If timestamp is duplicate, it's not inserted */
            timestamp_set.insert(timestamp);
        }
    }

    tstamps_size = timestamp_set.size();
    total_steps = tstamps_size * d_ptr->dataMap.size();
    steps_to_estimate = tstamps_size * 0.05;
    if(steps_to_estimate == 0)
        steps_to_estimate = tstamps_size;

    printf("\e[1;32m*\e[0m final data size will be %ld for each source... steps_to_estimate %ld\n",
           tstamps_size, steps_to_estimate);

    /* for each data row */
    for(std::map<std::string, std::list<XVariant> >::iterator it = d_ptr->dataMap.begin();
        it != d_ptr->dataMap.end(); ++it)
    {
        std::set<double>::iterator ts_set_iterator = timestamp_set.begin();
        /* take the vector of data from the map */
        std::list<XVariant> &data = it->second;
        /* create an iterator over data */
        std::list<XVariant>::iterator datait = data.begin();
        datait++; /* point to element 1 */

        while(datait != data.end())
        {
            step++;
            /* end of interval */
            tv1 = datait->getTimevalTimestamp();
            data_timestamp_1 = tv1.tv_sec + tv1.tv_usec * 1e-6;
            /* start of interval */
            tv0 = (--datait)->getTimevalTimestamp();
            data_timestamp_0 = tv0.tv_sec + tv0.tv_usec * 1e-6;
            datait++;
            /* iterate over the timestamps stored in the timestamp set. As we walk the set, avoid
             * searching the same interval multiple times. For this, keep ts_set_iterator as
             * start and update it in the last else if branch.
             */
            std::set<double>::iterator tsiter = ts_set_iterator;
            while(tsiter != timestamp_set.end())
            {
                if((*tsiter) >  data_timestamp_0 && (*tsiter) < data_timestamp_1)
                {
                    XVariant xv(*datait);
                    xv.setTimestamp((*tsiter));
                    datait = data.insert(datait, xv);
                }
                else if(*tsiter == data_timestamp_1) /* simply skip */
                {
//                    printf("\e[1;35m - skipping element cuz equal to timestamp_1: %s\e[0m", ctime(&tt));
                    tsiter++;
                    ts_set_iterator = tsiter; /* point to next */
                    break;
                }
                else if((*tsiter) > data_timestamp_1)
                {
                  //  tsiter++;
                    ts_set_iterator = tsiter; /* save to optimize next for */
//                    printf("\e[1;32m > going to next point after %s \e[0m", ctime(&tt));
                    break;
                }
                tsiter++;
                ts_set_iterator = tsiter; /* save to optimize next for */
            }
            datait++;
//            printf("\e[1;34m data index %ld data size %ld data %p\e[0m\n", dataidx, data.size(), &data);
            if(step % steps_to_estimate == 0)
            {
                double time_remaining = mEstimateFillTimeRemaining(&started_tv, step, total_steps);
                if(step == steps_to_estimate)
                    printf("\e[1;32m* \e[0mestimated time remaining: %.3fs\n", time_remaining);
            }
        }
    }
    gettimeofday(&ended_tv, NULL);
    d_ptr->elapsedMicros = ended_tv.tv_sec + ended_tv.tv_usec * 1e-6 -
            started_tv.tv_sec - started_tv.tv_usec * 1e-6;
    printf("\e[1;32m* \e[0mfilled in: %.3fs\n", d_ptr->elapsedMicros);
}

/** \brief returns the number of different sources dug out by divide
 *
 * @return the number of sources in the data.
 *
 * @see getSize
 * @see getSources
 */
size_t DataSiever::getSize() const
{
    return d_ptr->dataMap.size();
}

/** \brief tests whether there is data associated to the source specified
 *
 * @param source the name of the sought after source (attribute).
 *
 * @return true if DataSiever contains data for the source specified, false otherwise.
 *
 * @see getSize
 * @see getSources
 */
bool DataSiever::contains(std::string source) const
{
    return d_ptr->dataMap.count(source) > 0;
}

/** \brief Returns the list of sources stored by DataSiever
 *
 * @return a vector of std::string containing the sources stored by DataSiever.
 *
 *
 * @see getSize
 * @see getSources
 */
std::vector<std::string> DataSiever::getSources() const
{
    std::vector<std::string> srcs;
    for(std::map<std::string, std::list<XVariant> >::iterator it = d_ptr->dataMap.begin();
        it != d_ptr->dataMap.end(); ++it)
        srcs.push_back(it->first);
    return srcs;
}

/** \brief Returns the data associated to the provided source as a std::list
 *
 * @param source the name of the source (attribute) that contains the desired data
 * @return a std::list of XVariant associated to the given source. Each XVariant represents
 *         data at a certain time.
 *
 * \note You can retrieve data multiple times.
 * \note Data is stored by DataSiever as long as DataSiever is not destroyed.
 *
 */
std::list<XVariant> DataSiever::getDataAsList(std::string source) const
{
    std::list<XVariant>  ret;
    if(d_ptr->dataMap.count(source) > 0)
        ret = d_ptr->dataMap[source];
    return ret;
}

/** \brief Returns the data associated to the provided source
 *
 * @param source the name of the source (attribute) that contains the desired data
 * @return a vector of XVariant associated to the given source. Each XVariant represents
 *         data at a certain time.
 *
 * \note You can retrieve data multiple times.
 * \note Data is stored by DataSiever as long as DataSiever is not destroyed.
 *
 */
std::vector<XVariant> DataSiever::getData(std::string source) const
{
    struct timeval started_tv, ended_tv;
    gettimeofday(&started_tv, NULL);
    std::vector<XVariant> ret;
    if(d_ptr->dataMap.count(source) > 0)
        ret.insert(ret.begin(), d_ptr->dataMap[source].begin(), d_ptr->dataMap[source].end());
    gettimeofday(&ended_tv, NULL);

    printf("\e[1;33m* \e[0mgetData: converted to vector in: %.3fs\n", ended_tv.tv_sec + ended_tv.tv_usec * 1e-6 -
           started_tv.tv_sec - started_tv.tv_usec * 1e-6);
    return ret;
}

/** \brief Returns a reference to the internal raw std::map that associates a source to its data
 *
 */
const std::map<std::string, std::list<XVariant> > & DataSiever::getDataRef() const
{
    const std::map<std::string, std::list<XVariant> > &rData = d_ptr->dataMap;
    return rData;
}

/** \brief Returns the interna raw std::map that associates a source to its data
 *
 */
std::map<std::string, std::list<XVariant> > DataSiever::getData() const
{
    return d_ptr->dataMap;
}

double DataSiever::mEstimateFillTimeRemaining(struct timeval *start, int step, int total)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    double elapsed = now.tv_sec + now.tv_usec * 1e-6 - (start->tv_sec + start->tv_usec * 1e-6);
    double needed = ((double)(total - step)) * elapsed / (double) step;
    // printf("elapsed %f step %d total %d\n", elapsed, step, total);
    /* hypothesis: inserts are the time consuming operations. If vectors are the same size,
     * the resulting filled vector will be about twice.
     */
    return needed / d_ptr->dataMap.size();
}

/** \brief Add a DataSieverProgressListener in order to be notified when the fill operation
 *         changes its progress.
 *
 * @param a pointer to an object implementing the DataSieverProgressListener interface.
 *
 * @see removeDataSieverProgressListener
 */
void DataSiever::installDataSieverProgressListener(DataSieverProgressListener *dspl)
{
    d_ptr->dataSieverProgressListeners.push_back(dspl);
}

/** \brief Remove the specified DataSieverProgressListener in order to remove fill progress notifications.
 *
 * @param a pointer to DataSieverProgressListener previously inserted with
 *        installDataSieverProgressListener.
 */
void DataSiever::removeDataSieverProgressListener(DataSieverProgressListener *dspl)
{
    d_ptr->dataSieverProgressListeners.remove(dspl);
}
