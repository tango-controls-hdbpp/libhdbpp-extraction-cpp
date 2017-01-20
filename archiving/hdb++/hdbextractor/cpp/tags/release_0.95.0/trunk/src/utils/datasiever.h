#ifndef DATAFILLER_H
#define DATAFILLER_H

#include <vector>
#include <list>
#include <string>
#include <map>
#include <xvariant.h>

class DataSieverPrivate;
class DataSieverProgressListener;

/** \brief This class is an utility that is capable of performing two tasks: dividing the result of
 *         a multiple-source query by coupling the source name with the related data and filling the
 *         data of each source so that each array of the attribute's data has the same
 *         values on the x axis (timestamps).
 *
 * DataSiever <em>sieves</em> data allowing for:
 * \li dividing data by source name: a vector of XVariant belonging to distinct sources is sieved
 *     and data is grouped by source. You must call the divide method supplying a vector of XVariant
 *     containing the raw mixed data. Subsequently, calls to getData permit to obtain a vector of
 *     XVariant belonging to a particular source;
 *
 * \li making the <em>x axes</em> of each data equal. The x points (time) result from the union
 *     of the timestamps of every point in each source.
 *
 * For example, suppose that source s has data at t0, t4 and t9 and source t has data at t1,t3,t4 and t8.
 * Using the Hdbextractor to get data from sources s and t will return a vector of XVariant with the
 * points of s and t together.
 * The divide method will separate s from t.
 * The fill method will make s and t time scales equal. s and t will have points at times t0,t1,t3,t4,t8 and t9.
 * The values of s in t1,t2 and t3 will be the same as the value at t0 and the value of s in t8 will be the same
 * as the value in t4, and so on.
 * This time filling is useful if you want to deal with data having the same x dimension.
 *
 * \note divide must be called before fill because the former creates the internal map that stores the
 * data associated to each source and the latter uses the map.
 *
 * \note the vector of XVariant passed to divide must be the result of a Hdbextractor::getData
 *
 * \note You can retrieve data with getData multiple times.
 *
 * \note Data is stored by DataSiever and made available through getData as long as DataSiever is not destroyed.
 */
class DataSiever
{
public:

    DataSiever();

    virtual ~DataSiever();

    void divide(const std::vector<XVariant> &rawdata);

    void fill();

    void clear();

    double getElapsedTimeMicrosecs() const;

    size_t getSize() const;

    std::vector<std::string> getSources() const;

    std::list<XVariant> getDataAsList(std::string source) const;

    std::vector<XVariant> getData(std::string source) const;

    const std::map<std::string, std::list<XVariant> > &getDataRef() const;

    std::map<std::string, std::list<XVariant> > getData() const;

    bool contains(std::string source) const;

    void installDataSieverProgressListener(DataSieverProgressListener *dspl);

    void removeDataSieverProgressListener(DataSieverProgressListener *dspl);

private:
    DataSieverPrivate *d_ptr;

    double mEstimateFillTimeRemaining(struct timeval *start, int step, int total);
};

#endif // DATAFILLER_H
